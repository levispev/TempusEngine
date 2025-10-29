// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <unordered_set>
#include <functional>
#include <iostream>
#include <vector>
#include "Scene.h"
#include "Utils/EnumClassFlagUtils.h"

// Used for declaring a unique component type
// - Must have a component ID 
// - Must have a debug name declared with TPS_DEBUG_NAME()
// - Additional ComponentMetaData flags can be added as a third parameter
#define DECLARE_COMPONENT(type, id, ...) \
        TPS_STATIC_ASSERT(std::is_integral_v<decltype(id)>, "Component ID must be an integer"); \
        TPS_STATIC_ASSERT((id) >= 0 && (id) <= std::numeric_limits<Tempus::ComponentId>::max(), "Component ID must be a positive integer <= 255"); \
        public: \
        static constexpr ComponentId GetId() { return m_Id; } \
        static ComponentMetaData GetMetaData() { return m_MetaData; } \
        private: \
        static constexpr ComponentId m_Id = id; \
        static const inline ComponentMetaData m_MetaData = TPS_Private::ComponentRegistry::Register<type>(id, ##__VA_ARGS__);

namespace Tempus
{
    // Component meta data flags
    enum class ComponentMetaData : uint8_t
    {
        None = 0,
        // Disallow this component from being added in the editor
        NoEditorAdd = BIT(0),
        // Disallow this component from being serialized
        NoSerialize = BIT(1),
        // Restrict this component to only 1 per entity
        NoDuplicate = BIT(2)
    };
    ENUM_CLASS_FLAGS(ComponentMetaData);
}

namespace TPS_Private
{
    struct ComponentRegistry
    {
        struct ComponentTypeInfo
        {
            std::string name;
            Tempus::ComponentId id;
            Tempus::ComponentMetaData metadata = Tempus::ComponentMetaData::None;
            std::function<void(Tempus::Scene*, uint32_t)> addComponentFunc;
            std::function<void(Tempus::Scene*, uint32_t)> removeComponentFunc;
        };

        // I am unable to use the ValidComponent concept here as it's called before the class is fully created
        template<typename T>
        static Tempus::ComponentMetaData Register(Tempus::ComponentId id, Tempus::ComponentMetaData metadata = Tempus::ComponentMetaData::None)
        {
            // Component ID's must be unique
            if (ComponentIds.contains(id))
            {
                // Throwing an exception here instead of a critical log as the logger is not initialized yet
                throw std::runtime_error(std::format("Duplicate component ID's detected! {0}", id));
                return Tempus::ComponentMetaData::None;
            }

            // Insert new unique ID
            ComponentIds.insert(id);

            // Reflection data
            ComponentTypeInfo data;
            data.name = Tempus::TempusUtils::GetClassDebugName<T>();
            data.id = id;
            data.metadata = metadata;
            // Templated function pointer for adding the component to a scene
            data.addComponentFunc = [](Tempus::Scene* scene, uint32_t entityId)
            {   
                scene->AddComponent<T>(entityId);
            };
            data.removeComponentFunc = [](Tempus::Scene* scene, uint32_t entityId)
            {
              scene->RemoveComponent<T>(entityId);  
            };
            
            RegisteredComponents.emplace_back(data);
            ComponentMap[data.id] = data;

            return metadata;
        }

        static std::vector<std::string> GetRegisteredComponentNames()
        {
            std::vector<std::string> componentNames;
            for (const ComponentTypeInfo& info : RegisteredComponents)
            {
                componentNames.emplace_back(info.name);
            }
            return componentNames;
        }

        static const std::vector<ComponentTypeInfo>& GetRegisteredComponents()
        {
            return RegisteredComponents;
        }

        static ComponentTypeInfo GetComponentTypeFromId(Tempus::ComponentId id)
        {
            if (ComponentMap.contains(id))
            {
                return ComponentMap.at(id);
            }

            TPS_CRITICAL("Component ID [{0}] not found in registry!", id);
        }

        private:
        static inline std::vector<ComponentTypeInfo> RegisteredComponents;
        static inline std::map<Tempus::ComponentId, ComponentTypeInfo> ComponentMap;
        static inline std::unordered_set<Tempus::ComponentId> ComponentIds;
    };
}
