// Copyright Levi Spevakow (C) 2025

#include "Scene.h"
#include "Entities/Entity.h"
#include "Log.h"

Tempus::Scene::Scene(const std::string& sceneName) : m_SceneName(sceneName)
{
    for (uint32_t entity = 0; entity < MAX_ENTITIES; entity++)
    {
        m_AvailableEntityIds.push(entity); 
    }
}

Tempus::Entity Tempus::Scene::AddEntity(const std::string& name)
{
    if (m_EntityCount >= MAX_ENTITIES)
    {
        TPS_CORE_CRITICAL("Max entity count reached! Cannot create entity");
    }

    uint32_t id = m_AvailableEntityIds.front();
    m_AvailableEntityIds.pop();
    m_EntityCount++;

    Entity newEntity = Entity(id);
    newEntity.m_OwnerScene = this;
    newEntity.bActive = true;

    m_Entities.insert(id);
    m_EntityNames[id] = name;

    //@TODO std::move
    return newEntity;
}

