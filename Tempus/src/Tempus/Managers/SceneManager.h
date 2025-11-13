// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core/Core.h"
#include "Core/IUpdateable.h"
#include "Core/Scene.h"

#define SCENE_MANAGER ::Tempus::GApp->GetManager<Tempus::SceneManager>()

namespace Tempus
{
    class TEMPUS_API SceneManager : public IUpdateable
    {
        TPS_DEBUG_NAME("Scene Manager")
        
    private:

        friend class Application;

        SceneManager() = default;
        std::unique_ptr<Scene> m_ActiveScene = nullptr;

    public:
        
        Scene* CreateScene(const std::string& sceneName);
        Scene* GetActiveScene() const { return m_ActiveScene.get();}
        bool SetActiveScene(const std::string& sceneName);

        bool IsUpdating() const override { return true; };
        void OnUpdate(float DeltaTime) override;

    private:

        void CreateEditorCamera();
      
    };

}

