// Copyright Levi Spevakow (C) 2025

#pragma once

#include <glm/vec3.hpp>

#include "Core.h"

namespace Tempus
{

    class TEMPUS_API TransformComponent
    {
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

