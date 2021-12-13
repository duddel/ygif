#include "ygif_camera.h"

namespace mygame
{
    YgifTrafo *YgifCamera::trafo()
    {
        return (YgifTrafo *)(yourgame::math::Camera::trafo());
    }

    std::array<float, 6> YgifCamera::castRay(float viewportX, float viewportY)
    {
        glm::vec3 org, dir;
        yourgame::math::Camera::castRay(viewportX, viewportY, org, dir);
        return std::array<float, 6>{org.x, org.y, org.z, dir.x, dir.y, dir.z};
    }
}
