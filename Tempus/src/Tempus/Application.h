// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

#include "vulkan/vulkan.h"
#define SDL_MAIN_HANDLED

#include "sdl/SDL.h"

namespace Tempus {

	class TEMPUS_API Application
	{
	public:

		Application();
		virtual ~Application();
		void Run();

		static void RequestExit()
		{
			SDL_Event quitEvent;
			quitEvent.type = SDL_QUIT;
			SDL_PushEvent(&quitEvent);
		}

	protected:

		virtual void AppStart();
		virtual void Update();
		virtual void Cleanup();

		SDL_Event GetCurrentEvent() const { return CurrentEvent; }

		void SetRenderClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		class Window* GetWindow() const { return m_Window; }

	private:

		void InitWindow();
		void InitRenderer();
		void InitSDL();

		void CoreUpdate();
		void EventUpdate();

	private:

		VkInstance m_Instance = nullptr;
		class Window* m_Window = nullptr;
		class Renderer* m_Renderer = nullptr;

		bool bShouldQuit = false;
		SDL_Event CurrentEvent;
		
	protected:
		
		const char* AppName;

	};

	Application* CreateApplication();

}

