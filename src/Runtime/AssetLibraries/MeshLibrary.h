#pragma once
#include <stdint.h>
#include <Runtime/AssetLibraries/RenderResourceHandles.h>

struct ID3D12Device;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;

namespace mini
{
    enum class IndexFormat : uint8_t
    {
        R16_UINT,
        R32_UINT
    };

    static constexpr uint32_t GetIndexFormatStride(IndexFormat format)
    {
        return format == IndexFormat::R16_UINT ? sizeof(uint16_t) : sizeof(uint32_t);
    }

    struct  MeshResource;
    struct  MeshData
    {

        char*       vertexData = nullptr;
        char*       indexData = nullptr;
        uint32_t    vertexStride = 0;
        IndexFormat indexFormat = IndexFormat::R16_UINT;
        uint32_t    vertexDataSize = 0;
        uint32_t    indexDataSize = 0;
    };

    struct  MeshPool;

    struct ResourceID;

    class MeshLibrary
    {
        ID3D12Device*           m_device = nullptr;
        ID3D12DescriptorHeap*   m_srvHeap = nullptr;
        MeshPool*               m_pool = nullptr;
    public:

        bool                Initialize(ID3D12Device* device, uint32_t poolSize);

        MeshResourceHandle  Allocate(ResourceID const& resourceId) const;
        MeshResourceHandle  AllocateWithData(ResourceID const& resourceId, MeshData const& data);
        void                SetData(MeshResourceHandle handle, MeshData const& data) const;
        MeshResource const* Lookup(MeshResourceHandle handle) const;
        MeshResourceHandle  GetHandleForResourceId(ResourceID resourceId) const;

        void                Destroy(MeshResourceHandle);

    };


    struct MeshResource
    {
        uint64_t vertexBufferView = 0;
        uint64_t indexBufferView = 0;

        ID3D12Resource* vertexBufferResource = nullptr;
        ID3D12Resource* indexBufferResource = nullptr;

        uint32_t numIndices = 0;
        uint32_t numVertices = 0;
    };
}
