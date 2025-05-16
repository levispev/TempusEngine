// Copyright Levi Spevakow (C) 2025

#include "SceneManager.h"
#include "Components/Component.h"
#include "Entities/Entity.h"
#include "Components/TransformComponent.h"


std::unique_ptr<Tempus::SceneManager> Tempus::SceneManager::s_Instance = nullptr;

Tempus::Scene* Tempus::SceneManager::CreateScene(const std::string& sceneName)
{
    m_ActiveScene = new Scene(sceneName);
    return m_ActiveScene;
}

void Tempus::SceneManager::SetActiveScene(Scene* scene)
{
    m_ActiveScene = scene;
}

bool Tempus::SceneManager::SetActiveScene(const std::string& sceneName)
{
    return false;
}

void Tempus::SceneManager::DoTestScene()
{
    if (!m_ActiveScene)
    {
        return;
    }

    Entity e = m_ActiveScene->AddEntity("Test Entity1");
    e.AddComponent<TransformComponent>();
    Entity e1 = m_ActiveScene->AddEntity("Test Entity2");
    e.AddComponent<TransformComponent>();
    Entity e2 = m_ActiveScene->AddEntity("Test Entity3");
    e.AddComponent<TransformComponent>();
    Entity e3 = m_ActiveScene->AddEntity("Test Entity4");
    e.AddComponent<TransformComponent>();
    Entity e4 = m_ActiveScene->AddEntity("Test Entity5");
    e.AddComponent<TransformComponent>();
}
