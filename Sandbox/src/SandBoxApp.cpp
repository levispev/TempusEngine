// Copyright Levi Spevakow (C) 2025

#include "Tempus.h"
#include "Tempus/Core/Scene.h"
#include "Tempus/Managers/SceneManager.h"
#include "Tempus/Entity/Entity.h"
#include <random>
#include "Components/TransformComponent.h"

class SandBox : public Tempus::Application
{
public:

	SandBox()
	{
		AppName = "Sandbox";
	}

	~SandBox() override = default;

	virtual void AppStart() override
	{
		if (Tempus::Scene* scene = SCENE_MANAGER->GetActiveScene())
		{
			for (int i = 0; i < 5; i++)
			{
				Tempus::Entity e = scene->AddEntity("Test Entity" + std::to_string(i));
				e.AddComponent<Tempus::TransformComponent>(glm::vec3(1.0f));

				if(Tempus::TransformComponent* tc = e.GetComponent<Tempus::TransformComponent>())
				{
	            
				}
			}
		}
	}

	virtual void Update() override
	{
		SDL_Event event = GetCurrentEvent();

		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_Y)
			{
				TPS_WARN("Color Change!");

				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_int_distribution<> dis(0, 255);

				SetRenderClearColor(dis(gen), dis(gen), dis(gen), 255);

				if (Tempus::Scene* scene = SCENE_MANAGER->GetActiveScene())
				{
					for (int i = 0; i < 100; i++)
					{
						Tempus::Entity e = scene->AddEntity("Entity_" + std::to_string(i));
						e.AddComponent<Tempus::TransformComponent>();
					}
				}
			}
		}
	}

};

Tempus::Application* Tempus::CreateApplication()
{
	return new SandBox();
}
