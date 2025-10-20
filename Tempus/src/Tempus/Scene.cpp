// Copyright Levi Spevakow (C) 2025

#include "Scene.h"
#include "Entity/Entity.h"
#include "Log.h"

void Tempus::Scene::OnUpdate(float DeltaTime)
{
    m_SceneTime += DeltaTime;
}

Tempus::Scene::Scene(std::string sceneName) : m_SceneName(std::move(sceneName))
{
    for (uint32_t entity = 0; entity < MAX_ENTITIES; entity++)
    {
        m_AvailableEntityIds.push(entity); 
    }
}

Tempus::Entity Tempus::Scene::AddEntity(std::string name)
{
    TPS_ASSERT(m_EntityCount < MAX_ENTITIES, "Max entity count reached! Cannot create entity");

    uint32_t id = m_AvailableEntityIds.front();
    m_AvailableEntityIds.pop();
    m_EntityCount++;

    Entity newEntity = Entity(id);
    newEntity.m_OwnerScene = this;
    newEntity.bActive = true;

    TPS_CORE_TRACE("Entity Created! Name: [{0}] ID: [{1}]", name, id);
    
    m_Entities.insert(id);
    m_EntityNames[id] = std::move(name);
    
    return newEntity;
}

void Tempus::Scene::RemoveEntity(uint32_t id)
{
    TPS_ASSERT(m_EntityCount < MAX_ENTITIES, "Cannot remove entity of ID [{0}]. ID exceeds maximum!", id);

    if (!m_EntityNames.contains(id))
    {
        TPS_CORE_ERROR("Cannot remove entity of ID [{0}]. Does not exist!", id);
        return;
    }
    
    m_AvailableEntityIds.push(id);
    m_EntityCount--;

    m_Entities.erase(id);
    m_EntityNames.erase(id);

    // Remove all components from all pools for this entity
    for (auto& [componentId, pool] : m_ComponentPools)
    {
        pool->RemoveComponent(id);
    }

    m_EntityComponents[id].reset();
    
    TPS_CORE_TRACE("Entity Removed! ID: [{0}]", id);
}

std::string Tempus::Scene::GetEntityName(uint32_t id)
{
    if (m_EntityNames.contains(id))
    {
        return m_EntityNames[id];
    }

    TPS_CORE_ERROR("Entity with ID [{0}] does not exist!", id);
    return {};
}
