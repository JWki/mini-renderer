#pragma once

#include "Resource.h"
#include <EASTL/fixed_function.h>
#include <Runtime/util.h>

namespace mini
{
    enum class ResourceLoadResult
    {
        Success, FileNotFound, Cached, OutOfMemory
    };

    using ResourceHandler = eastl::fixed_function<sizeof(void*) * 4, void(Resource*)>;

    template <int NUM_RESOURCES>
    class ResourceManager
    {
        Resource m_resources[NUM_RESOURCES];
        uint32_t m_numResources = 0;

        ResourceHandler m_resourceHandlers[static_cast<int>(ResourceType::_LastType)];

    public:
        
        void RegisterResourceHandler(ResourceType resourceType, ResourceHandler handler);

        ResourceLoadResult LoadResource(char const* filePath, ResourceID id, Resource** outResource = nullptr);
    };
}


/**/
template <int NUM_RESOURCES>
void mini::ResourceManager<NUM_RESOURCES>::RegisterResourceHandler(ResourceType resourceType, ResourceHandler handler)
{
    m_resourceHandlers[static_cast<int>(resourceType)] = handler;
}

template <int NUM_RESOURCES>
mini::ResourceLoadResult mini::ResourceManager<NUM_RESOURCES>::LoadResource(char const* filePath, ResourceID id, Resource** outResource)
{
    if (m_numResources == NUM_RESOURCES) { return ResourceLoadResult::OutOfMemory; }

    ResourceInfo info;
    info.type = ResourceType::Mesh;
    info.file.path = filePath;
    info.id = id;
    auto const resourceData = static_cast<char*>(Win32LoadFileContents(filePath, &info.file.size));
    if(!resourceData)
    {
        return ResourceLoadResult::FileNotFound;
    }
    m_resources[m_numResources++] = Resource(info, resourceData);
    auto resourcePtr = &m_resources[m_numResources - 1];
    if(m_resourceHandlers[static_cast<int>(info.type)] != nullptr)
    {
        m_resourceHandlers[static_cast<int>(info.type)](resourcePtr);
    }
    if (outResource != nullptr) { *outResource = resourcePtr; }
    return ResourceLoadResult::Success;
}