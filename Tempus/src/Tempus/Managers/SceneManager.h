// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include "IUpdateable.h"
#include "Scene.h"

#define SCENE_MANAGER Tempus::GApp->GetManager<Tempus::SceneManager>()

namespace Tempus
{
    class TEMPUS_API SceneManager : public IUpdateable
    {

        TPS_DEBUG_NAME("Scene Manager")
        
    private:

        friend class Application;

        SceneManager() = default;
        Scene* m_ActiveScene = nullptr;

    public:
        
        Scene* CreateScene(const std::string& sceneName);
        Scene* GetActiveScene() const { return m_ActiveScene;}
        void SetActiveScene(Scene* scene);
        bool SetActiveScene(const std::string& sceneName);

        bool IsUpdating() const override { return true; };
        void OnUpdate(float DeltaTime) override;

        // Testing function
        void DoTestScene();

    private:

        void CreateEditorCamera();
      
    };

}

