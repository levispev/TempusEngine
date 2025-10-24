// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Application.h"
#include "Core.h"

namespace Tempus
{
    class TEMPUS_API IUpdateable
    {
    public:

        IUpdateable()
        {
            if (GApp)
            {
                GApp->RegisterUpdateable(this);
            }
        }
        
        virtual ~IUpdateable()
        {
            if (GApp)
            {
                GApp->UnregisterUpdateable(this);
            }
        }
        
        virtual void OnUpdate(float DeltaTime) = 0;
    
    };

}
