#ifndef YGIF_CAMERA_H
#define YGIF_CAMERA_H

#include <array>
#include "yourgame/math/camera.h"
#include "ygif_trafo.h"

namespace mygame
{
    class YgifCamera : yourgame::math::Camera
    {
    public:
        YgifTrafo *trafo();
        std::array<float, 6> castRay(float viewportX, float viewportY);
    };
}

#endif
