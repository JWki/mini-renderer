#include "MeshLibrary.h"
#include <Runtime/common.h>
#include <Runtime/Resources/Resource.h>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRA_LEAN
#define NOMINMAX
#include <d3d12.h>

#pragma warning(push, 0)
#include <Runtime/par_shapes-h.h>
#pragma warning(pop)

// @todo use generational handle instead of straight index to detect use after free
struct mini::MeshPool
{
    struct Element
    {
        MeshResource    resource;
        bool            isUsed      = false;
        ResourceID      resourceId; 
    }                   *elements   = nullptr;;
    uint32_t            size        = 0;
};

bool mini::MeshLibrary::Initialize(ID3D12Device* device, uint32_t poolSize)
{
    m_device = device;

    m_pool = new MeshPool;
    m_pool->elements = new MeshPool::Element[poolSize];
    m_pool->size = poolSize;

    // create a descriptor heap for vertex and index buffer SRVs 
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = poolSize * 2;     // @note see notes for MeshLibrary::SetData
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;   // @note this heap doesn't need to be shader visible because descriptors are copied into a ringbuffer at render time
    desc.NodeMask = 0;
    auto res = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap));
    MINI_ASSERT(SUCCEEDED(res), "Failed to create SRV descriptor heap");
    if (FAILED(res)) {
        return false;
    }


    // @note reserve first element for a fallback mesh so missing resources are visually represented in the scene
    m_pool->elements[0].isUsed = true;  
    {
        char* meshDataBuf = nullptr;   // @note we share an allocation for vertices and indices
        mini::MeshData meshData;
        static_assert(sizeof(uint16_t) == sizeof(PAR_SHAPES_T));

        {   // create a procedural cube mesh, copy out positions and normals 
            auto mesh = par_shapes_create_cube();
            par_shapes_translate(mesh, -0.5f, -0.5f, -0.5f);
            par_shapes_unweld(mesh, true);
            par_shapes_compute_normals(mesh);

            meshData.vertexDataSize = sizeof(float) * 3 * (mesh->npoints * 2);
            meshData.vertexStride = sizeof(float) * 6;

            meshData.indexDataSize = sizeof(uint16_t) * 3 * mesh->ntriangles;

            meshData.vertexData = meshDataBuf = reinterpret_cast<char*>(malloc(meshData.vertexDataSize + meshData.indexDataSize));
            for (auto i = 0; i < mesh->npoints; ++i) {

                auto writePtr = meshData.vertexData + meshData.vertexStride * i;
                memcpy(writePtr, mesh->points + i * 3, sizeof(float) * 3);
                writePtr += sizeof(float) * 3;
                memcpy(writePtr, mesh->normals + i * 3, sizeof(float) * 3);
            }
            meshData.indexData = meshDataBuf + meshData.vertexDataSize;
            memcpy(meshData.indexData, mesh->triangles, meshData.indexDataSize);
        }

        SetData({ 0 }, meshData);
        free(meshDataBuf);  // @note we can free our mesh data here since we don't have a reason to keep it around any longer
    }

   
    //
    return true;
}


mini::MeshResourceHandle mini::MeshLibrary::Allocate(ResourceID const& resourceId) const
{
    for(auto i = 0u; i < m_pool->size; ++i)
    {
        if(m_pool->elements[i].isUsed == false)
        {
            m_pool->elements[i].isUsed = true;
            m_pool->elements[i].resourceId = resourceId;
            return { i };   
        }
    }
    return MeshResourceHandle();    // @note index 0 is reserved for error case
}

mini::MeshResourceHandle mini::MeshLibrary::AllocateWithData(ResourceID const& resourceId, MeshData const& data)
{
    auto handle = Allocate(resourceId);
    MINI_ASSERT(handle.handle != 0, "Failed to allocate resource slot");
    SetData(handle, data);
    return handle;
}

mini::MeshResource const* mini::MeshLibrary::Lookup(MeshResourceHandle handle) const
{
    // @todo assert generation here
    if (!m_pool->elements[handle.handle].isUsed) { return nullptr; }
    return &m_pool->elements[handle.handle].resource;
}

mini::MeshResourceHandle mini::MeshLibrary::GetHandleForResourceId(ResourceID resourceId) const
{
    for(auto i = 0u; i < m_pool->size; ++i)
    {
        if(m_pool->elements[i].resourceId == resourceId)
        {
            return { i };
        }
    }
    return MeshResourceHandle();
}


void mini::MeshLibrary::SetData(MeshResourceHandle handle, MeshData const& data) const
{
    auto& resource = m_pool->elements[handle.handle].resource;

    resource.numVertices = data.vertexDataSize / data.vertexStride;
    resource.numIndices = data.indexDataSize / GetIndexFormatStride(data.indexFormat);

    {   // Vertex Buffer
        // @todo    Use a staging buffer for uploading and issue a GPU copy into GPU exclusive memory 
        //          instead of using an upload heap for the vertex data 

        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = data.vertexDataSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        // @note we allow the runtime to manage our memory allocation here
        auto res = m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource.vertexBufferResource));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create commited resource for vertex buffer");


        D3D12_RANGE readRange = {};
        void* map = nullptr;
        res = resource.vertexBufferResource->Map(0, &readRange, &map);
        MINI_ASSERT(SUCCEEDED(res), "Failed to map vertex buffer");
        memcpy(map, data.vertexData, data.vertexDataSize);
        resource.vertexBufferResource->Unmap(0, 0);
    }

    {   // Index buffer
        // @todo    Use a staging buffer for uploading and issue a GPU copy into GPU exclusive memory 
        //          instead of using an upload heap for the index data
        // @todo    Share an allocation between vertex and index data

        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = data.indexDataSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        // @note we allow the runtime to manage our memory allocation here
        auto res = m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource.indexBufferResource));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create commited resource for index buffer");


        D3D12_RANGE readRange = {};
        void* map = nullptr;
        res = resource.indexBufferResource->Map(0, &readRange, &map);
        MINI_ASSERT(SUCCEEDED(res), "Failed to map index buffer");
        memcpy(map, data.indexData, data.indexDataSize);
        resource.indexBufferResource->Unmap(0, 0);
    }


    auto srvStartCPU = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    auto const incrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_CPU_DESCRIPTOR_HANDLE vertexBufferSRVCPU = srvStartCPU;

    // @note we allocate descriptors in pairs of 2 (one for vertex and index buffer each), using the index of the resource as an index into the descriptor pool 
    vertexBufferSRVCPU.ptr += incrSize * 2 * handle.handle;
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = data.vertexDataSize / data.vertexStride;
        desc.Buffer.StructureByteStride = data.vertexStride;
        m_device->CreateShaderResourceView(resource.vertexBufferResource, &desc, vertexBufferSRVCPU);
    }
    D3D12_CPU_DESCRIPTOR_HANDLE indexBufferSRVCPU = vertexBufferSRVCPU;
    indexBufferSRVCPU.ptr += incrSize;
    {
        DXGI_FORMAT indexFormats[] = {
            DXGI_FORMAT_R16_UINT,
            DXGI_FORMAT_R32_UINT
        };

        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        desc.Format = indexFormats[static_cast<int>(data.indexFormat)];
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = resource.numIndices;
        desc.Buffer.StructureByteStride = 0;
        m_device->CreateShaderResourceView(resource.indexBufferResource, &desc, indexBufferSRVCPU);
    }

    // @note we use the CPU descriptor here because we copy these on the CPU timeline into a per-frame ringbuffer at render time
    resource.vertexBufferView = vertexBufferSRVCPU.ptr;
    resource.indexBufferView = indexBufferSRVCPU.ptr;
}


