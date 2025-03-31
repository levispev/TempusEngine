// Copyright Levi Spevakow (C) 2025

#include "Tempus.h"

#include <random>

class SandBox : public Tempus::Application
{
public:

	SandBox() 
	{
	}

	~SandBox() 
	{
	}

	virtual void Update() override
	{
		
		SDL_Event event = GetCurrentEvent();

		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_A)
			{

				TPS_WARN("Color Change!");

				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_int_distribution<> dis(0, 255);

				SetRenderClearColor(dis(gen), dis(gen), dis(gen), 255);

			}
		}

	}

};

Tempus::Application* Tempus::CreateApplication()
{
	return new SandBox();
}