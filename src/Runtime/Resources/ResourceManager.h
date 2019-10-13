#pragma once

#include "Resource.h"
#include <EASTL/fixed_function.h>


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

        ResourceLoadResult LoadResource(char const* filePath, Resource* outResource);
    };
}


/**/
template <int NUM_RESOURCES>
void mini::ResourceManager<NUM_RESOURCES>::RegisterResourceHandler(mini::ResourceType resourceType, mini::ResourceHandler handler)
{
    m_resourceHandlers[static_cast<int>(resourceType)] = handler;
}

template <int NUM_RESOURCES>
mini::ResourceLoadResult mini::ResourceManager<NUM_RESOURCES>::LoadResource(char const* filePath, mini::Resource* outResource)
{
    if (m_numResources == NUM_RESOURCES) { return ResourceLoadResult::OutOfMemory; }
    mini::ResourceInfo info;
    info.type = 
    info.file.path = filePath;
    info.file.size = 0;
    m_resources[m_numResources++] = Resource(info, resourceData);
}