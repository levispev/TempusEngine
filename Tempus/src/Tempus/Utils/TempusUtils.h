// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core/Core.h"

namespace Tempus
{

    class TEMPUS_API TempusUtils {

    public:

        template<typename T>
        static constexpr const char* GetClassDebugName()
        {
#ifndef TPS_DIST
            TPS_STATIC_ASSERT(requires {T::DebugName; }, "Class must have an implemented debug name with TPS_DEBUG_NAME()");
            return T::DebugName;
#else
            return "Debug Name";
#endif
        }
    };

}