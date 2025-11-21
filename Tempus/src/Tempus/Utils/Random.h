// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core/Core.h"
#include <random>

namespace Tempus
{
    class TEMPUS_API Random
    {
    public:

        // Random number in the range [min, max]. Int: inclusive, Float: inclusive-exclusive
        template<typename T>
        static T RandRange(T min, T max)
        {
            static_assert(std::is_arithmetic_v<T>, "RandRange requires an arithmetic type.");

            if constexpr (std::is_integral_v<T>)
            {
                std::uniform_int_distribution<T> dist(min, max);
                return dist(GetGenerator());
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                std::uniform_real_distribution<T> dist(min, max);
                return dist(GetGenerator());
            }
        }

        // Random double in range [0.0, 1.0]
        static float RandFloat()
        {
            return RandRange(0.0f, 1.0f);
        }

        // Random double in range [0.0, 1.0]
        static double RandDouble()
        {
            return RandRange(0.0, 1.0);
        }

        // Random int in full range of int32_t
        static int RandInt()
        {
            return std::uniform_int_distribution<int>{}(GetGenerator());
        }

        // Random unsigned in range [0 - UINT_MAX]
        static uint32_t RandUInt()
        {
            return std::uniform_int_distribution<uint32_t>{}(GetGenerator());
        }

        // Random int in full range of int64_t
        static int64_t RandInt64()
        {
            return std::uniform_int_distribution<int64_t>{}(GetGenerator());
        }

    private:

        static std::mt19937& GetGenerator()
        {
            thread_local std::mt19937 generator(std::random_device{}());
            return generator;
        }
    };
    
}

