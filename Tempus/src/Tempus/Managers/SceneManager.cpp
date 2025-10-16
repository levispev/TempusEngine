// Copyright Levi Spevakow (C) 2025

#include "SceneManager.h"
#include "Components/Component.h"
#include "Entity/Entity.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"


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

    Entity e = m_ActiveScene->AddEntity("Editor Cam");
    e.AddComponent<CameraComponent>();

    for (int i = 0; i < 5; i++)
    {
        Entity e = m_ActiveScene->AddEntity("Test Entity" + std::to_string(i));
        e.AddComponent<TransformComponent>();

        if(TransformComponent* tc = e.GetComponent<TransformComponent>())
        {
            
        }
    }

}
