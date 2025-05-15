// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <queue>
#include <array>
#include <bitset>
#include <map>
#include <set>
#include <string>

namespace Tempus
{
    class Component;
    using ComponentSignature = std::bitset<MAX_COMPONENTS>;
    
    class TEMPUS_API Scene
    {
    public:

        class Entity AddEntity(const std::string& name);
        
        template<typename T, typename ...Args>
        requires std::derived_from<T, Component>
        void AddComponent(uint32_t id, Args... arguments)
        {
            
        }

        inline uint32_t GetEntityCount() const { return m_EntityCount; }
        inline const std::string& GetName() const { return m_SceneName; }
        std::vector<std::string> GetEntityNames()
        {
            std::vector<std::string> names;
            for (const auto& pair: m_EntityNames)
            {
                names.push_back(pair.second);
            }
            return names;
        }

    private:

        friend class SceneManager;
        
        Scene(const std::string& sceneName);
        ~Scene();
        
        std::queue<uint32_t> m_AvailableEntityIds;
        std::array<ComponentSignature, MAX_ENTITIES> m_EntityComponents;
        std::map<uint32_t, std::string> m_EntityNames;
        std::set<uint32_t> m_Entities;
        uint32_t m_EntityCount = 0;

        std::string m_SceneName;
        
    };
    
}
