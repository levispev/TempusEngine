// Copyright Levi Spevakow (C) 2025

#include "Tempus.h"

#include <random>
#include <chrono>

class SandBox : public Tempus::Application
{
public:

	std::chrono::time_point<std::chrono::high_resolution_clock> lastTime = std::chrono::high_resolution_clock::now();
	double deltaTime = 0.0;
	double fps = 0.0;

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

				TPS_WARN("Colour Change!");

				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_int_distribution<> dis(0, 255);

				SetRenderColor(dis(gen), dis(gen), dis(gen), 255);

			}
		}


		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
		lastTime = currentTime;

		fps = 1.0 / deltaTime;

		//TPS_INFO(fps);

		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

};

Tempus::Application* Tempus::CreateApplication()
{
	return new SandBox();
}