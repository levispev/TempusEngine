// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include "Component.h"

namespace Tempus
{
    class TEMPUS_API StaticMeshComponent : public Component
    {
        DECLARE_COMPONENT(StaticMeshComponent, 2)
        TPS_DEBUG_NAME("Static Mesh Component")
    };
}

