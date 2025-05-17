// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"

// Used for declaring unique ID for a component type
// Every component must have a component ID
#define DECLARE_COMPONENT_ID(x) public:\
                                static constexpr uint8_t GetId() { return m_Id; }\
                                private:\
                                static constexpr uint8_t m_Id = x;\
                                TPS_STATIC_ASSERT((x) >= 0);

namespace Tempus
{
    class TEMPUS_API Component
    {
        DECLARE_COMPONENT_ID(0)

    private:

        friend class Scene;
        Component();

    protected:
        
        virtual ~Component() = default;
    
    };
    
}
