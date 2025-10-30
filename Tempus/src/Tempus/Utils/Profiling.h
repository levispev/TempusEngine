// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

#define TPS_PROFILE(label) ::Tempus::Profiling::ScopedProfiler scopedProfiler##__LINE__ = { label }; scopedProfiler##__LINE__.Start()

namespace Tempus
{
    class TEMPUS_API Profiling
    {

    public:

        struct ProfilingData
        {
            ProfilingData(const char* inLabel, float inDuration) : label(inLabel), duration(inDuration)
            {
            }
            const char* label = nullptr;
            float duration = 0.0f;
        };
        
        struct ScopedProfiler
        {
            ScopedProfiler(const char* inLabel) : label(inLabel)
            {
            }

            ~ScopedProfiler()
            {
                if (bStarted)
                {
                    auto end =  std::chrono::high_resolution_clock::now();
                    float duration = std::chrono::duration<float, std::milli>(end - start).count();
                    RegisterProfileData({ label, duration });
                }
            }

            void Start()
            {
                start = std::chrono::high_resolution_clock::now();
                bStarted = true;
            }

            ScopedProfiler(const ScopedProfiler&) = delete;
            ScopedProfiler& operator=(const ScopedProfiler&) = delete;

            const char* label = nullptr;

        private:
            std::chrono::time_point<std::chrono::steady_clock> start;
            bool bStarted = false;
        };

        static void RegisterProfileData(const ProfilingData& data)
        {
            s_ProfilingData.emplace_back(data);
        }

        static std::vector<ProfilingData>& GetProfilingData()
        {
            return s_ProfilingData;
        }

        static void Flush()
        {
            s_ProfilingData.erase(s_ProfilingData.begin(), s_ProfilingData.end());
        }

    private:

        static inline std::vector<ProfilingData> s_ProfilingData;
        
    };
    
}