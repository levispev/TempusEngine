// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

// Internal helper macros for selecting proper scoped profiler label
#define TPS_PROFILE_SELECT(_1, _2, NAME, ...) NAME
#define TPS_PROFILE_0() ::Tempus::Profiling::ScopedProfiler TPS_MACRO_JOIN(scopedProfiler, __LINE__)(__func__)
#define TPS_PROFILE_1(label) ::Tempus::Profiling::ScopedProfiler TPS_MACRO_JOIN(scopedProfiler, __LINE__)(label)

// Macro for executing a scoped profiler trace.
// Can optionally be given a label, if no label is entered it will use the function name.
// If using multiple times within the same function, a label should be provided to distinguish instances.
#define TPS_PROFILE(...) TPS_PROFILE_SELECT(dummy, ##__VA_ARGS__, TPS_PROFILE_1, TPS_PROFILE_0)(__VA_ARGS__)

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
                start = std::chrono::high_resolution_clock::now();
            }

            ~ScopedProfiler()
            {
                auto end =  std::chrono::high_resolution_clock::now();
                float duration = std::chrono::duration<float, std::milli>(end - start).count();
                RegisterProfileData({ label, duration });
            }

            ScopedProfiler(const ScopedProfiler&) = delete;
            ScopedProfiler& operator=(const ScopedProfiler&) = delete;

            const char* label = nullptr;

        private:
            std::chrono::time_point<std::chrono::steady_clock> start;
        };

        static void RegisterProfileData(ProfilingData data)
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