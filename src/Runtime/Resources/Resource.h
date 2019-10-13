#pragma once
#include <stdint.h>

namespace mini
{
    enum class ResourceType : uint8_t
    {
        Undefined,
        Mesh, Shader, Texture2D, Material, _LastType
    };

    struct ResourceID
    {
        uint32_t value = 0;
        bool operator == (ResourceID const& other) const { return value == other.value; }
    };

    struct ResourceInfo
    {
        ResourceType    type = ResourceType::Undefined;
        ResourceID      id;
        struct {
            char const* path = "";
            uint64_t    size = 0;
        } file;
    };

    class Resource
    {
        ResourceInfo    m_info;
        char*           m_rawData = nullptr;
    public:
        Resource() = default;
        Resource(ResourceInfo info, char* data) : m_info(info), m_rawData(data) {}
        
        ResourceInfo const& GetInfo() const { return m_info; }
        char const* const   GetData() const { return m_rawData; }
    };
}