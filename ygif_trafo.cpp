#include "ygif_trafo.h"

namespace mygame
{
    void YgifTrafo::rotateGlobal(float angle, std::string const &ax)
    {
        if (ax == "X")
        {
            yourgame::math::Trafo::rotateGlobal(angle, yourgame::math::Axis::X);
        }
        else if (ax == "Y")
        {
            yourgame::math::Trafo::rotateGlobal(angle, yourgame::math::Axis::Y);
        }
        else if (ax == "Z")
        {
            yourgame::math::Trafo::rotateGlobal(angle, yourgame::math::Axis::Z);
        }
    }

    void YgifTrafo::rotateLocal(float angle, std::string const &ax)
    {
        if (ax == "X")
        {
            yourgame::math::Trafo::rotateLocal(angle, yourgame::math::Axis::X);
        }
        else if (ax == "Y")
        {
            yourgame::math::Trafo::rotateLocal(angle, yourgame::math::Axis::Y);
        }
        else if (ax == "Z")
        {
            yourgame::math::Trafo::rotateLocal(angle, yourgame::math::Axis::Z);
        }
    }

    void YgifTrafo::translateLocal(glm::vec3 const &trans)
    {
        yourgame::math::Trafo::translateLocal(trans);
    }

    void YgifTrafo::translateGlobal(glm::vec3 const &trans)
    {
        yourgame::math::Trafo::translateGlobal(trans);
    }

    void YgifTrafo::setScaleLocal(glm::vec3 const &scale)
    {
        yourgame::math::Trafo::setScaleLocal(scale);
    }
}
