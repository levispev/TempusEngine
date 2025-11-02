// Copyright Levi Spevakow (C) 2025

#include "SceneManager.h"

#include "Core/Application.h"
#include "Components/Component.h"
#include "Entity/Entity.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Components/EditorDataComponent.h"
#include "Components/StaticMeshComponent.h"

Tempus::Scene* Tempus::SceneManager::CreateScene(const std::string& sceneName)
{
    m_ActiveScene = std::make_unique<Scene>(sceneName);
    
    CreateEditorCamera();
    
    return m_ActiveScene.get();
}

bool Tempus::SceneManager::SetActiveScene(const std::string& sceneName)
{
    return false;
}

void Tempus::SceneManager::OnUpdate(float DeltaTime)
{
    if (m_ActiveScene)
    {
        m_ActiveScene->OnUpdate(DeltaTime);
    }
}

void Tempus::SceneManager::DoTestScene()
{
    if (!m_ActiveScene)
    {
        return;
    }

    for (int i = 0; i < 5; i++)
    {
        Entity e = m_ActiveScene->AddEntity("Test Entity" + std::to_string(i));
        e.AddComponent<TransformComponent>();
        e.AddComponent<StaticMeshComponent>();

        if(TransformComponent* tc = e.GetComponent<TransformComponent>())
        {
            tc->Position = glm::vec3(i * 2.0f, 0.0f, 0.0f);
        }
    }

}

void Tempus::SceneManager::CreateEditorCamera()
{
    if (m_ActiveScene)
    {
        Entity editorCam = m_ActiveScene->AddEntity("Editor Camera");
        editorCam.AddComponent<TransformComponent>(glm::vec3(-5.0f, 0.0f, 0.0f));
        editorCam.AddComponent<CameraComponent>();
        editorCam.GetComponent<TransformComponent>()->Position = glm::vec3(-5.0f, 0.0f, 0.0f);
        editorCam.AddComponent<EditorDataComponent>(EditorEntityDataFlags::NoDelete | EditorEntityDataFlags::NoSerialize | EditorEntityDataFlags::NoAddComponent | EditorEntityDataFlags::NoRemoveComponent);
    }
}
