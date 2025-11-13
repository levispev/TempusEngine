// Copyright Levi Spevakow (C) 2025

#include "Tempus.h"
#include "Tempus/Core/Scene.h"
#include "Tempus/Managers/SceneManager.h"
#include "Tempus/Entity/Entity.h"
#include <random>

#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Utils/Time.h"

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
				// for (int i = 0; i < 3; i++)
				// {
				// 	for (int j = 0; j < 3; j++)
				// 	{
				// 		Entity e = scene->AddEntity("Test Entity" + std::to_string(i * 3 + j));
				// 		e.AddComponent<TransformComponent>(glm::vec3(1.0f));
				// 		e.AddComponent<StaticMeshComponent>();
				//
				// 		if(TransformComponent* tc = e.GetComponent<TransformComponent>())
				// 		{
				// 			tc->Position = glm::vec3((i * 2.0f) - 2, (j * 2.0f) - 2, 0.0f);
				// 		}
				// 	}	
				// }
				//
				// Entity bigCube = scene->AddEntity("Big Cube");
				// bigCubeId = bigCube.GetId();
				// bigCube.AddComponent<TransformComponent>(glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(5.0f));
				// bigCube.AddComponent<StaticMeshComponent>();
				//
				// if(TransformComponent* tc = bigCube.GetComponent<TransformComponent>())
				// {
				// 	tc->Position = glm::vec3(20.0f, 0.0f, 0.0f);
				// }

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

		virtual void AppUpdate() override
		{
			SDL_Event event = GetCurrentEvent();

			// if (Scene* scene = SCENE_MANAGER->GetActiveScene())
			// {
			// 	if (scene->HasEntity(bigCubeId))
			// 	{
			// 		if (TransformComponent* transComp = scene->GetComponent<TransformComponent>(bigCubeId))
			// 		{
			// 			transComp->Position.z = sinf((float)Time::GetSceneTime()) * 5.0f;

			// 			transComp->Rotation.z += 20.0f * Time::GetDeltaTime();
			// 			if (transComp->Rotation.z > 360.0f)
			// 			{
			// 				transComp->Rotation.z = 0.0f;
			// 			}
			// 		}
			// 	}
			// }
			
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_Y)
				{
					TPS_WARN("Color Change!");

					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_int_distribution<> dis(0, 255);

					SetRenderClearColor(dis(gen), dis(gen), dis(gen), 255);

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

	};
	
}


Tempus::Application* Tempus::CreateApplication()
{
	return new SandBox();
}
