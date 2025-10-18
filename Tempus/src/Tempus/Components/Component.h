// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"
#include "ComponentRegistry.h"

// Used for declaring a unique component type
// - Must have a component ID 
// - Must have a debug name declared with TEMPUS_DEBUG_NAME()
#define DECLARE_COMPONENT(type, id) \
        TPS_STATIC_ASSERT(std::is_integral_v<decltype(id)>, "Component ID must be an integer"); \
        TPS_STATIC_ASSERT((id) >= 0 && (id) <= std::numeric_limits<Tempus::ComponentId>::max(), "Component ID must be a positive integer <= 255"); \
        public: \
        static constexpr ComponentId GetId() { return m_Id; } \
        private: \
        static constexpr ComponentId m_Id = id; \
        static const inline bool _bIsComponentRegistered = TPS_Private::ComponentRegistry::Register<type>(id);

namespace Tempus
{
    
    class TEMPUS_API Component
    {
        TPS_DEBUG_NAME("Component")

    protected:
        
        Component();
        virtual ~Component();

    };
    
}