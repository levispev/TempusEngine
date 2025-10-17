// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <unordered_set>
#include <functional>
#include <vector>
#include "Scene.h"

namespace TPS_Private
{
    struct ComponentRegistry
    {
        public:

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
            static std::unordered_set<Tempus::ComponentId> ids;
            if (ids.contains(id)) 
            {
                // Throwing an exception here instead of a critical log as the logger is not initialized yet
                throw std::runtime_error(std::format("Duplicate component ID's detected! {0}", id));
            }

            ids.insert(id);

            ComponentTypeInfo data;
            data.name = Tempus::TempusUtils::GetClassDebugName<T>();
            data.id = id;
            data.defaultConstructor = [](Tempus::Scene* scene, uint32_t entityId)
            {   
                scene->AddComponent<T>(entityId);
            };
            
            RegisteredComponents.push_back(data);

            return true;
        }

        static std::vector<std::string> GetRegisteredComponents()
        {
            std::vector<std::string> componentNames;
            for (ComponentTypeInfo info : RegisteredComponents)
            {
                componentNames.push_back(info.name);
            }
            return componentNames;
        }

        private:
        static std::vector<ComponentTypeInfo> RegisteredComponents;
    };
}