// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core/Core.h"
#include "System.h"

namespace Tempus
{
    class TEMPUS_API EditorCameraSystem : public System
    {
    public:

        EditorCameraSystem();
        void OnInit(class Scene* ownerScene) override;
        void OnUpdate(float DeltaTime) override;
        
    };
}
