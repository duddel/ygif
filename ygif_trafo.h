#ifndef YGIF_TRAFO_H
#define YGIF_TRAFO_H

#include <string>
#include "yourgame/math/trafo.h"

namespace mygame
{
    class YgifTrafo : yourgame::math::Trafo
    {
    public:
        void rotateGlobal(float angle, std::string const &ax);
        void rotateLocal(float angle, std::string const &ax);
        void translateLocal(glm::vec3 const &trans);
        void translateGlobal(glm::vec3 const &trans);
        void setScaleLocal(glm::vec3 const &scale);
    };
}

#endif
