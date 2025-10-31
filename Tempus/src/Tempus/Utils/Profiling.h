// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

#ifndef TPS_DIST
// Macro for executing a scoped timer trace.
// Can optionally be given a label.
#define TPS_SCOPED_TIMER(...) ::Tempus::Profiling::ScopedProfiler TPS_MACRO_JOIN(scopedProfiler, __LINE__)(FUNC_NAME, ##__VA_ARGS__)
#else
#define TPS_SCOPED_TIMER(...)
#endif

namespace Tempus
{
    class TEMPUS_API Profiling
    {
    public:

        struct ProfilingData
        {
            ProfilingData(const char* inFuncName, double inDuration, const char* inLabel) : functionName(inFuncName), duration(inDuration), label(inLabel)
            {
            }
            const char* functionName = nullptr;
            double duration = 0.0f;
            const char* label = nullptr;
        };
        
        struct ScopedProfiler
        {
            ScopedProfiler(const char* inFuncName, const char* inLabel = nullptr) : functionName(inFuncName) , label(inLabel)
            {
                start = std::chrono::high_resolution_clock::now();
            }

            ~ScopedProfiler()
            {
                auto end =  std::chrono::high_resolution_clock::now();
                double duration = std::chrono::duration<double, std::milli>(end - start).count();
                RegisterProfileData({ functionName, duration, label });
            }

            ScopedProfiler(const ScopedProfiler&) = delete;
            ScopedProfiler& operator=(const ScopedProfiler&) = delete;

            const char* functionName = nullptr;
            const char* label = nullptr;

        private:
            std::chrono::time_point<std::chrono::steady_clock> start;
        };

        static void RegisterProfileData(ProfilingData data)
        {
            s_ProfilingData.emplace_back(data);
        }

        static const std::vector<ProfilingData>& GetProfilingData()
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