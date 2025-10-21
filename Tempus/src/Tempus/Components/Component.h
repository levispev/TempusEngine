// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"
#include "ComponentRegistry.h"

namespace Tempus
{
    class TEMPUS_API Component
    {
        TPS_DEBUG_NAME("Component")

    protected:
        
        Component() = default;
        virtual ~Component() = default;
        virtual void OnDrawImGui() {};

    };
}