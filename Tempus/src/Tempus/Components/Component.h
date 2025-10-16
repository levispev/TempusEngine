// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"
#include <unordered_set>
#include <iostream>
#include <limits>

namespace Tempus
{
    using ComponentId = uint8_t;
}

namespace TPS_Private
{
    struct ComponentRegistry
    {
        static bool Register(Tempus::ComponentId id)
        {
            static std::unordered_set<Tempus::ComponentId> ids;
            if (ids.contains(id)) 
            {
                // Throwing an exception here instead of a critical log as the logger is not initialized yet
                throw std::runtime_error(std::format("Duplicate component ID's detected! {0}", id));
            }

            ids.insert(id);
            
            return true;
        }
    };
}


// Used for declaring a unique component type
// Every component must have a component ID 
#define DECLARE_COMPONENT_ID(x) \
        TPS_STATIC_ASSERT(std::is_integral_v<decltype(x)>, "Component ID must be an integer"); \
        TPS_STATIC_ASSERT((x) >= 0 && (x) <= std::numeric_limits<Tempus::ComponentId>::max(), "Component ID must be a positive integer <= 255"); \
        private: \
        static inline bool _bIsComponentRegistered = TPS_Private::ComponentRegistry::Register(x); \
        public: \
        static constexpr ComponentId GetId() { return m_Id; } \
        private: \
        static constexpr ComponentId m_Id = x; \

namespace Tempus
{
    class Component;

    // Concept for valid component
    // Constraint #1: Must derive from Component class
    // Constraint #2: Must implement valid ID using 'DECLARE_COMPONENT_ID(x)'
    template<typename T>
    concept ValidComponent = std::derived_from<T, Component> && requires {{T::GetId()} -> std::convertible_to<ComponentId>;};
    
    class TEMPUS_API Component
    {

    protected:
        
        Component() = default;
        virtual ~Component() = default;

    };
    
}