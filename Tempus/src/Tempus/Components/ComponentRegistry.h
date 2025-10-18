// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <unordered_set>
#include <functional>
#include <iostream>
#include <vector>
#include "Scene.h"

namespace TPS_Private
{
    struct ComponentRegistry
    {
        struct ComponentTypeInfo
        {
            std::string name;
            Tempus::ComponentId id;
            std::function<void(Tempus::Scene*, uint32_t)> defaultConstructor;
        };

        // I am unable to use the ValidComponent concept here as it's called before the class is fully created
        template<typename T>
        static bool Register(Tempus::ComponentId id)
        {
            // Component ID's must be unique
            if (ComponentIds.contains(id))
            {
                // Throwing an exception here instead of a critical log as the logger is not initialized yet
                throw std::runtime_error(std::format("Duplicate component ID's detected! {0}", id));
            }

            // Insert new unique ID
            ComponentIds.insert(id);

            // Reflection data
            ComponentTypeInfo data;
            data.name = Tempus::TempusUtils::GetClassDebugName<T>();
            data.id = id;
            // Templated function pointer for adding the component to a scene
            data.defaultConstructor = [](Tempus::Scene* scene, uint32_t entityId)
            {   
                scene->AddComponent<T>(entityId);
            };
            
            RegisteredComponents.emplace_back(data);

            return true;
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

        private:
        static inline std::vector<ComponentTypeInfo> RegisteredComponents;
        static inline std::unordered_set<Tempus::ComponentId> ComponentIds;
    };
}