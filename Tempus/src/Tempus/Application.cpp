// Copyright Levi Spevakow (C) 2024

#include "Application.h"
#include <thread>
#include "Log.h"

namespace Tempus {

	Tempus::Application::Application()
	{
		
	}

	Tempus::Application::~Application()
	{
	}

	void Application::Run()
	{

		Log::Init();

		int num = 42;
		TPS_CORE_TRACE("Cool number: {0}", num);

		SDL_SetMainReady();

		SDL_Init(SDL_INIT_VIDEO);

		//SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

		m_Window = SDL_CreateWindow("Cool Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);

		SDL_Renderer* renderer = SDL_CreateRenderer(m_Window, -1, 0);

		SDL_SetRenderDrawColor(renderer, 19, 61, 102, 255);

		SDL_Event event;

		TPS_CORE_TRACE("Trace Log");
		TPS_CORE_INFO("Info Log");
		TPS_CORE_WARN("Warn Log");
		TPS_CORE_ERROR("Error Log");
		TPS_CORE_CRITICAL("Critical Log");

		while (true) {

			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);

			SDL_PollEvent(&event);

			if (event.type == SDL_QUIT) {
				break;
			}
			
		}

	}


}

