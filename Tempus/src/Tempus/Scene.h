// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <queue>
#include <array>
#include <bitset>
#include <map>
#include <set>
#include <string>
#include "Log.h"
#include "Components/Component.h"

namespace Tempus
{
    using ComponentSignature = std::bitset<MAX_COMPONENTS>;
    
    class TEMPUS_API Scene
    {
    public:

        class Entity AddEntity(const std::string& name);
        
        template<ValidComponent T, typename ...Args>
        void AddComponent(uint32_t id, Args... arguments)
        {
            m_EntityComponents[id].set(T::GetId());
            TPS_CORE_TRACE("Component added of ID [{0}] added to entity [{1}]", T::GetId(), m_EntityNames[id]);
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

        template<ValidComponent T>
        T* GetComponent(uint32_t id)
        {
            if(HasComponent<T>(id))
            {
                //@TODO create component pools to fetch from
                return nullptr;
            }
            return nullptr;
        }

        template<ValidComponent T>
        bool HasComponent(uint32_t id)
        {
            if(m_EntityComponents[id].test(T::GetId()))
            {
                return true;
            }

            return false;
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
