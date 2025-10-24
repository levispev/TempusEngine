// Copyright Levi Spevakow (C) 2025

#include "EditorCameraSystem.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"

Tempus::EditorCameraSystem::EditorCameraSystem() : System()
{
    // Initializing the signature for the Editor Camera System
    m_Signature.set(TransformComponent::GetId());
    m_Signature.set(CameraComponent::GetId());
}

void Tempus::EditorCameraSystem::OnInit(class Scene* ownerScene)
{
    System::OnInit(ownerScene);
}

void Tempus::EditorCameraSystem::OnUpdate(float DeltaTime)
{
    
}
