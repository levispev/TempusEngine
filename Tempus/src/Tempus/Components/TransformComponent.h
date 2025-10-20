// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <glm/vec3.hpp>
#include "Component.h"

namespace Tempus
{

    class TEMPUS_API TransformComponent : public Component
    {
        DECLARE_COMPONENT(TransformComponent, 0)
        TPS_DEBUG_NAME("Transform Component")

    public:

        TransformComponent() = default;
        TransformComponent(glm::vec3 position) : Position(position) {}
        TransformComponent(glm::vec3 position, glm::vec3 rotation) : Position(position), Rotation(rotation) {}
        TransformComponent(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : Position(position), Rotation(rotation), Scale(scale) {}
        
        // May want to remove the union as the vec3 layout isnt 100% guaranteed
        union
        {
            glm::vec3 Position = glm::vec3(0.0f);
            struct
            {
                float x;
                float y;
                float z;
            };
        };

        glm::vec3 Rotation = glm::vec3(0.0f);
        glm::vec3 Scale = glm::vec3(0.0f);

        glm::vec3 GetForwardVector() const;
        glm::vec3 GetRightVector() const;
        glm::vec3 GetUpVector() const;
        
    };
    
}

