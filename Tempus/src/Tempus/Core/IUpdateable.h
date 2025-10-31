// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

namespace Tempus
{
    class TEMPUS_API IUpdateable
    {
    public:

        IUpdateable() = default;
        virtual ~IUpdateable() = default;
        virtual void OnUpdate(float DeltaTime) {}
        virtual bool IsUpdating() const { return false; }
    };
}
