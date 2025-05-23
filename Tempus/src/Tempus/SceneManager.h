// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <memory>
#include "Scene.h"

#define SCENE_MANAGER SceneManager::GetInstance()

namespace Tempus
{
    class TEMPUS_API SceneManager
    {
    private:
        SceneManager() = default;
        static std::unique_ptr<SceneManager> s_Instance;

        Scene* m_ActiveScene = nullptr;

    public:

        static SceneManager* GetInstance()
        {
            if (!s_Instance)
            {
                // Not using std::make_unique because of private constructor
                s_Instance = std::unique_ptr<SceneManager>(new SceneManager());
            }
            return s_Instance.get();
        }

        Scene* CreateScene(const std::string& sceneName);
        inline Scene* GetActiveScene() const { return m_ActiveScene;}
        void SetActiveScene(Scene* scene);
        bool SetActiveScene(const std::string& sceneName);

        // Testing function
        void DoTestScene();
        
    };

}
