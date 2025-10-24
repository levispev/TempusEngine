// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include "IUpdateable.h"

namespace Tempus
{
    class TEMPUS_API System : IUpdateable
    {
    public:

        virtual ~System() = default;
        virtual void OnInit() {}
        virtual void OnUpdate(float DeltaTime) override {}
    
    };

}
