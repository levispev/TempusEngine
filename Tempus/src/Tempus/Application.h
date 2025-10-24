// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

#include "vulkan/vulkan.h"
#define SDL_MAIN_HANDLED

#include <bitset>
#include <unordered_set>
#include "sdl/SDL.h"

namespace Tempus {
	
	class IUpdateable;
	class SceneManager;
	class Window;
	class Renderer;

	class TEMPUS_API Application
	{
	public:

		Application();
		virtual ~Application();
		void Run();
		
		SceneManager* GetSceneManager() const { return m_SceneManager; }

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
		Window* GetWindow() const { return m_Window; }

	private:

		void InitWindow();
		void InitRenderer();
		void InitSDL();
		void InitManagers();

		void CoreUpdate();
		void ManagerUpdate();
		void EventUpdate();

		// Temporary input 
		void ProcessInput(SDL_Event event);
		void UpdateEditorCamera();
		static inline std::bitset<10> m_InputBits;
		static inline std::map<int, int> m_InputMap = 
		{ {SDL_SCANCODE_W, 0}, {SDL_SCANCODE_A, 1}, {SDL_SCANCODE_S, 2}, {SDL_SCANCODE_D, 3},
		  {SDL_SCANCODE_Q, 4}, {SDL_SCANCODE_E, 5}, {SDL_SCANCODE_UP, 6}, {SDL_SCANCODE_DOWN, 7},
		  {SDL_SCANCODE_RIGHT, 8}, {SDL_SCANCODE_LEFT, 9}
		};

	private:

		VkInstance m_Instance = nullptr;
		Window* m_Window = nullptr;
		Renderer* m_Renderer = nullptr;

		// Managers
		SceneManager* m_SceneManager = nullptr;

		bool bShouldQuit = false;
		SDL_Event CurrentEvent;

		static inline std::unordered_set<IUpdateable*> m_Managers;
		
	protected:
		
		const char* AppName;

	};

	Application* CreateApplication();

}

