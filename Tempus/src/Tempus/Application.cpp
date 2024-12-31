// Copyright Levi Spevakow (C) 2024

#include "Application.h"
#include <thread>
#include "Log.h"
#include <random>

#include "Window.h"
#include "Renderer.h"

namespace Tempus {

	Application::Application()
	{
		m_Window = new Window();
		m_Renderer = new Renderer();
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{

		Log::Init();

		// SDL Initialization
		SDL_SetMainReady();

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) 
		{
			TPS_CORE_CRITICAL("Failed to initialize SDL: {0}", SDL_GetError());
			return;
		}

		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

		// Window creation
		if (!InitWindow()) 
		{
			return;
		}
		// Renderer creation
		if (!InitRenderer()) 
		{
			return;
		}

		SDL_Event event;

		while (true) {

			SDL_PollEvent(&event);

			if (event.type == SDL_QUIT) {
				break;
			}
			else if (event.type == SDL_KEYDOWN) {
				
				if (event.key.keysym.scancode == SDL_SCANCODE_A) {

					TPS_WARN("Colour Change!");

					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_int_distribution<> dis(0, 255);

					m_Renderer->SetRenderDrawColor(dis(gen), dis(gen), dis(gen), 255);
				}
			}

			m_Renderer->RenderClear();
			m_Renderer->RenderPresent();

		}
	}

	bool Application::InitWindow()
	{
		// Window creation
		if (!m_Window || !m_Window->Init("Cool Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN))
		{
			TPS_CORE_CRITICAL("Failed to initialize window!");
			return false;
		}

		TPS_CORE_INFO("Window successfully created!");

		return true;

	}

	bool Application::InitRenderer()
	{

		// Renderer creation
		if (!m_Renderer || !m_Renderer->Init(m_Window))
		{
			TPS_CORE_CRITICAL("Failed to initialize renderer!");
			return false;
		}

		m_Renderer->SetRenderDrawColor(19, 16, 102, 255);

		TPS_CORE_INFO("Renderer successfully created!");

		return true;
	}
}