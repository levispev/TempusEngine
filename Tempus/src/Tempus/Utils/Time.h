// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <memory>

namespace Tempus
{

    class TEMPUS_API Time
    {
    public:

        static Time* GetInstance();
        static float GetDeltaTime();
        static float GetAppTime();
        static float GetTimeScale();
        static void SetTimeScale(float scale);
        // @TODO Make proper event system and have this automatically calculate on application tick event
        // For now it's unsafely public and manually being called by application
        void CalculateDeltaTime();

    private:

        Time();
        
        static std::unique_ptr<Time> s_Instance;
        
        static float m_DeltaTime;
        static float m_AppTime;
        static float m_TimeScale;
        
    };

    
}
