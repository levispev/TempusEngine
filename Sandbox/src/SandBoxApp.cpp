// Copyright Levi Spevakow (C) 2025

#include "Tempus.h"
#include "Tempus/Core/Scene.h"
#include "Tempus/Managers/SceneManager.h"
#include "Tempus/Entity/Entity.h"
#include "Tempus/Utils/Random.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"

namespace Tempus
{
	class SandBox : public Tempus::Application
	{
	public:

		SandBox()
		{
			AppName = "Sandbox";
		}

		~SandBox() override = default;

		uint32_t bigCubeId;

		virtual void AppStart() override
		{
			if (Scene* scene = SCENE_MANAGER->GetActiveScene())
			{
				Entity e = scene->AddEntity("Fbx Test1");
				e.AddComponent<TransformComponent>();
				e.AddComponent<StaticMeshComponent>();

				Entity e1 = scene->AddEntity("Fbx Test2");
				e1.AddComponent<TransformComponent>(glm::vec3(-175.0f, 0.0f, 0.0f));
				e1.AddComponent<StaticMeshComponent>();

				Entity e2 = scene->AddEntity("Fbx Test2");
				e2.AddComponent<TransformComponent>(glm::vec3(175.0f, 0.0f, 0.0f));
				e2.AddComponent<StaticMeshComponent>();
			}
		}

		virtual void AppEvent(const SDL_Event& event) override
		{
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				if (event.key.scancode == SDL_SCANCODE_Y)
				{
					TPS_WARN("Color Change!");
					
					SetRenderClearColor(Random::RandRange(0, 255), Random::RandRange(0, 255), Random::RandRange(0, 255), 255);

					if (Scene* scene = SCENE_MANAGER->GetActiveScene())
					{
						for (int i = 0; i < 100; i++)
						{
							Entity e = scene->AddEntity("Entity_" + std::to_string(i));
							e.AddComponent<TransformComponent>();
						}
					}
				}
			}
		}

		virtual void AppUpdate() override
		{
			
		}

	};
	
}


Tempus::Application* Tempus::CreateApplication()
{
	return new SandBox();
}
