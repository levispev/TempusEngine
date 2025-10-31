// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core/Core.h"
#include <memory>

namespace Tempus
{
    class TEMPUS_API Time
    {
    public:

        static Time* GetInstance();
        static float GetDeltaTime();
        static float GetUnscaledDeltaTime();
        static double GetAppTime();
        static float GetTimeScale();
        static double GetSceneTime();
        static void SetTimeScale(float scale);
        static void CalculateDeltaTime();

    private:

        Time();
        
        static std::unique_ptr<Time> s_Instance;
        
        static float m_DeltaTime;
        static float m_UnscaledDeltaTime;
        static double m_AppTime;
        static float m_TimeScale;
    };
}
