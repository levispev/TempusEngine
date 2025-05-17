// Copyright Levi Spevakow (C) 2025

#include "Utils/Time.h"
#include <time.h>

#include "Events/EventDispatcher.h"
#include "Events/IEventListener.h"

std::unique_ptr<Tempus::Time> Tempus::Time::s_Instance = nullptr;

float Tempus::Time::m_DeltaTime = 0.0f;
float Tempus::Time::m_Time = 0.0f;
float Tempus::Time::m_TimeScale = 1.0f;

Tempus::Time* Tempus::Time::GetInstance()
{
    if (!s_Instance)
    {
        s_Instance = std::unique_ptr<Time>(new Time());
    }
    return s_Instance.get();
}

float Tempus::Time::GetDeltaTime()
{
    return m_DeltaTime;
}

float Tempus::Time::GetTime()
{
    return m_Time;
}

float Tempus::Time::GetTimeScale()
{
    return m_TimeScale;   
}

void Tempus::Time::SetTimeScale(float scale)
{
    m_TimeScale = scale;
}

Tempus::Time::Time()
{
    
}

void Tempus::Time::CalculateDeltaTime()
{
    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    // Calculate deltatime
    m_DeltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - lastFrameTime).count();
    // Update current time since application start
    m_Time += m_DeltaTime;
    
    lastFrameTime = currentTime;
}
