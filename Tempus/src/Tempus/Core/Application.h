// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

#include "vulkan/vulkan.h"
#define SDL_MAIN_HANDLED

#include <typeindex>
#include <bitset>
#include <unordered_set>
#include "SDL3/SDL.h"
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

		static void RequestExit(const std::string& reason = "None given")
		{
			SDL_Event quitEvent;
			quitEvent.type = SDL_EVENT_QUIT;
			SDL_PushEvent(&quitEvent);
			TPS_INFO("Exit requested: {0}", reason);
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

		float GetMouseX() const { return m_LastMouseX; }
		float GetMouseY() const { return m_LastMouseY; }
		float GetMouseDeltaX() const { return m_MouseDeltaX; }
		float GetMouseDeltaY() const { return m_MouseDeltaY; }

	protected:

		virtual void AppStart();
		virtual void AppUpdate();
		virtual void AppEvent(const SDL_Event& event) {};
		virtual void Cleanup();

		void SetRenderClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		Window* GetWindow() const { return m_Window.get(); }

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
		void ProcessMouseMovement(SDL_Event event);
		void UpdateEditorCamera();
		static inline std::bitset<8> m_InputBits;
		float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
		float m_MouseDeltaX = 0.0f, m_MouseDeltaY = 0.0f;
		float m_SavedMouseX = 0.0f, m_SavedMouseY = 0.0f;
		float m_SavedMouseScrolls = 0.0f;
		float m_MouseSensitivity;
		uint8_t m_CatchMouseButton;
		float m_EditorCamSpeed = 10.0f;
		static inline std::map<int, int> m_InputMap = 
		{ {SDL_SCANCODE_W, 0}, {SDL_SCANCODE_A, 1}, {SDL_SCANCODE_S, 2}, {SDL_SCANCODE_D, 3},
		  {SDL_SCANCODE_Q, 4}, {SDL_SCANCODE_E, 5}, {SDL_SCANCODE_LSHIFT, 6}, {SDL_SCANCODE_LCTRL, 7}
		};

	private:

		VkInstance m_Instance = nullptr;
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Renderer> m_Renderer;

		bool bShouldQuit = false;
		SDL_Event CurrentEvent;

		static inline std::unordered_map<std::type_index, IUpdateable*> m_Managers;
		
	protected:
		
		const char* AppName;

	};

	Application* CreateApplication();

}

