// Copyright Levi Spevakow (C) 2025

#pragma once

#include <glm/vec3.hpp>
#include "Component.h"
#include "Core.h"

namespace Tempus
{

    class TEMPUS_API TransformComponent : public Component
    {

        DECLARE_COMPONENT_ID(1)

    public:

        union
        {
            glm::vec3 Position;
            struct
            {
                float x;
                float y;
                float z;
            };
        };
        
        glm::vec3 Rotation;
        glm::vec3 Scale;
        
    };
    
}

