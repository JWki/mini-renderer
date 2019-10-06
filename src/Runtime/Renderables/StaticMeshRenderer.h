#pragma once

#include <Runtime/AssetLibraries/RenderResourceHandles.h>
#include <Runtime/Math/math_types.h>

namespace mini
{
    struct Transform
    {
        math::vec3f_t   position;
        math::quatf_t   rotation;
        float           uniformScale;
    };

    struct StaticMesh
    {
        MeshResourceHandle  resourceHandle;
        Transform           transform;
    };

    struct StaticMeshHandle { uint32_t handle = 0; };

}