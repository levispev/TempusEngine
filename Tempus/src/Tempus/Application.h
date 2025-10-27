// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

#include "vulkan/vulkan.h"
#define SDL_MAIN_HANDLED

#include <typeindex>
#include <bitset>
#include <unordered_set>
#include "sdl/SDL.h"
#include "Utils/TempusUtils.h"

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

		static void RequestExit()
		{
			SDL_Event quitEvent;
			quitEvent.type = SDL_QUIT;
			SDL_PushEvent(&quitEvent);
		}

		template<typename T>
		T* GetManager()
		{
			auto it = m_Managers.find(std::type_index(typeid(T)));
			if (it != m_Managers.end())
			{
				return static_cast<T*>(it->second);
			}
			return nullptr;
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

		template<typename T>
		void CreateManager()
		{
			std::type_index typeIndex = std::type_index(typeid(T));
			if (!m_Managers.contains(typeIndex))
			{
				m_Managers[std::type_index(typeid(T))] = new T();
			}
			else
			{
				TPS_WARN("Attempted to create manager of type [{0}] but it already exists!", TempusUtils::GetClassDebugName<T>());
			}
		}

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

		bool bShouldQuit = false;
		SDL_Event CurrentEvent;

		static inline std::unordered_map<std::type_index, IUpdateable*> m_Managers;
		
	protected:
		
		const char* AppName;

	};

	Application* CreateApplication();

}

