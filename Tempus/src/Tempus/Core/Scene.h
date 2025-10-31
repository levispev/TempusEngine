// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <queue>
#include <array>
#include <bitset>
#include <map>
#include <set>
#include <string>
#include <memory>
#include "Log.h"
#include "Systems/System.h"
#include "Utils/TempusUtils.h"

namespace Tempus
{
    using ComponentSignature = std::bitset<MAX_COMPONENTS>;

    // Base class for type-erased component pools
    class IComponentPool
    {
    public:
        virtual ~IComponentPool() = default;
        virtual void RemoveComponent(uint32_t entityId) = 0;
    };

    // Templated component pool that stores components of a specific type
    template<ValidComponent T>
    class ComponentPool : public IComponentPool
    {
    public:
        ComponentPool()
        {
            // Initialize all optionals as empty
            m_ComponentArray.fill(std::nullopt);
        }

        template<typename... Args>
        void AddComponent(uint32_t entityId, Args&&... args)
        {
            // By this point the Scene class should have already checked for the component existence, checking again for safety
            if (!m_ComponentArray[entityId].has_value())
            {
                m_ComponentArray[entityId].emplace(std::forward<Args>(args)...);
            }
            else
            {
                TPS_ERROR("Component already exists for entity [{0}]!", entityId);
            }
        }

        void RemoveComponent(uint32_t entityId) override
        {
            if (entityId < MAX_ENTITIES)
            {
                m_ComponentArray[entityId].reset();
            }
        }

        T* GetComponent(uint32_t entityId)
        {
            if (entityId < MAX_ENTITIES && m_ComponentArray[entityId].has_value())
            {
                return &m_ComponentArray[entityId].value();
            }
            return nullptr;
        }

        bool HasComponent(uint32_t entityId) const
        {
            return entityId < MAX_ENTITIES && m_ComponentArray[entityId].has_value();
        }

    private:

        // @TODO This is poor for memory usage, this will be converted to a more efficient structure later
        std::array<std::optional<T>, MAX_ENTITIES> m_ComponentArray;
    };
    
    class TEMPUS_API Scene : public IUpdateable
    {
    public:

        Scene(std::string sceneName);
        ~Scene() = default;
        
        class Entity AddEntity(std::string name);
        void RemoveEntity(uint32_t id);

        std::vector<uint32_t> GetEntityIDs() { return std::vector(m_Entities.begin(), m_Entities.end()); }
        std::string GetEntityName(uint32_t id);
        uint32_t GetEntityCount() const { return m_EntityCount; }
        bool HasEntity(uint32_t id) const;
        const std::string& GetName() const { return m_SceneName; }
        float GetSceneTime() const { return m_SceneTime; }
        
        template<ValidComponent T, typename ...Args>
        void AddComponent(uint32_t id, Args&&... arguments)
        {
            if (!m_Entities.contains(id))
            {
                TPS_ERROR("Entity of ID [{0}] does not exist!", id);
                return;
            }
            
            ComponentId componentId = T::GetId();

            // Check if the entity already has the component
            if (m_EntityComponents[id].test(componentId))
            {
                TPS_ERROR("{1} already exists for entity [{0}]!", id, TempusUtils::GetClassDebugName<T>());
                return;
            }
            
            // Create pool if it doesn't exist
            if (!m_ComponentPools.contains(componentId))
            {
                m_ComponentPools[componentId] = std::make_unique<ComponentPool<T>>();
            }

            // Add component to the pool
            auto* pool = static_cast<ComponentPool<T>*>(m_ComponentPools[componentId].get());
            pool->AddComponent(id, std::forward<Args>(arguments)...);
            
            // Update signature
            m_EntityComponents[id].set(componentId);
            
            TPS_TRACE("{2} [{0}] added to entity [{1}]", componentId, m_EntityNames[id], TempusUtils::GetClassDebugName<T>());
        }
        
        std::vector<std::string> GetEntityNames() const
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
            if (!m_Entities.contains(id))
            {
                TPS_ERROR("Entity of ID [{0}] does not exist!", id);
                return nullptr;
            }
            
            ComponentId componentId = T::GetId();
            
            if (!m_ComponentPools.contains(componentId))
            {
                return nullptr;
            }

            auto* pool = static_cast<ComponentPool<T>*>(m_ComponentPools[componentId].get());
            return pool->GetComponent(id);
        }

        template<ValidComponent T>
        bool HasComponent(uint32_t id)
        {
            return m_EntityComponents[id].test(T::GetId());
        }

        template<ValidComponent T>
        void RemoveComponent(uint32_t id)
        {
            if (!m_Entities.contains(id))
            {
                TPS_ERROR("Entity of ID [{0}] does not exist!", id);
                return;
            }
            
            if (!HasComponent<T>(id))
            {
                TPS_ERROR("Cannot remove {1} from Entity [{0}], component not found!", m_EntityNames[id], TempusUtils::GetClassDebugName<T>());
                return;
            }
            
            ComponentId componentId = T::GetId();
            
            if (m_ComponentPools.contains(componentId))
            {
                auto* pool = static_cast<ComponentPool<T>*>(m_ComponentPools[componentId].get());
                pool->RemoveComponent(id);
                m_EntityComponents[id].reset(componentId);
                
                TPS_TRACE("{2} [{0}] removed from entity [{1}]", componentId, m_EntityNames[id], TempusUtils::GetClassDebugName<T>());
            }
        }
        
        ComponentId GetComponentCount(uint32_t id) const
        {
            return static_cast<ComponentId>(m_EntityComponents[id].count());
        }

        // Deleting copy and move operations since Scene owns unique resources
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;
    
    private:

        friend class SceneManager;

        void ResetSceneTime() { m_SceneTime = 0.0f; }

        bool IsUpdating() const override { return true; }
        void OnUpdate(float DeltaTime) override;
        
        std::queue<uint32_t> m_AvailableEntityIds;
        std::array<ComponentSignature, MAX_ENTITIES> m_EntityComponents;
        std::map<uint32_t, std::string> m_EntityNames;
        std::set<uint32_t> m_Entities;
        uint32_t m_EntityCount = 0;

        // Component pools indexed by component ID
        std::map<ComponentId, std::unique_ptr<IComponentPool>> m_ComponentPools;

        std::vector<std::unique_ptr<System>> m_Systems;
        
        std::string m_SceneName;

        float m_SceneTime = 0.0f;
        
    };
    
}
