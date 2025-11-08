// Copyright Levi Spevakow (C) 2025

#pragma once

#include <glm/vec3.hpp>

#include "Core/Core.h"
#include "Component.h"

namespace Tempus
{
    
    class TEMPUS_API LightComponent : public Component
    {
        DECLARE_COMPONENT(LightComponent, 4)
        TPS_DEBUG_NAME("Light Component")
        
    public:

        float Radius = 1.0f;
        float Intensity = 1.0f;
        glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
        
    };
}
