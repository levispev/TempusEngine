// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"
#include <unordered_set>
#include <iostream>

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


#define DECLARE_COMPONENT_ID(x) \
                        TPS_STATIC_ASSERT(std::is_integral_v<decltype(x)>, "Component ID must be an integer!"); \
                        TPS_STATIC_ASSERT((x) >= 0 && (x) <= 255, "Invalid component ID! Must be a positive integer <= 255"); \
                        private: \
                        static inline bool _bIsComponentRegistered = TPS_Private::ComponentRegistry::Register(x); \
                        public: \
                        static constexpr ComponentId GetId() { return m_Id; } \
                        private: \
                        static constexpr ComponentId m_Id = x; \

#ifndef TPS_DIST
    #define TPS_DEBUG_NAME(name) \
        public: \
        static const inline std::string DebugName = name; \
        private:
#else
    #define TPS_DEBUG_NAME(name)
#endif

// Used for declaring a unique component type
// Every component must have a component ID and a debug name
#define DECLARE_COMPONENT(id, name) \
        DECLARE_COMPONENT_ID(id)  \
        TPS_DEBUG_NAME(name)

namespace Tempus
{
    class Component;

    // Concept for valid component
    // Constraint #1: Must derive from Component class
    // Constraint #2: Must implement valid ID using 'DECLARE_COMPONENT_ID(x)'
    // Constraint #3 (DEBUG): Must have a debug name
#ifdef TPS_DEBUG
    template<typename T>
    concept ValidComponent = std::derived_from<T, Component> && requires {{T::GetId()} -> std::convertible_to<ComponentId>;} && requires {T::DebugName;};
#else
    template<typename T>
    concept ValidComponent = std::derived_from<T, Component> && requires {{T::GetId()} -> std::convertible_to<ComponentId>;};
#endif
    
    class TEMPUS_API Component
    {

    private:

        friend class Scene;
        Component();

    protected:
        
        virtual ~Component() = default;

    };
    
}