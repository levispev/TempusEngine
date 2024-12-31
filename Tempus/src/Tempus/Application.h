// Copyright Levi Spevakow (C) 2024

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

	private:

		bool InitWindow();
		bool InitRenderer();

	private:

		VkInstance m_Instance = nullptr;
		class Window* m_Window = nullptr;
		class Renderer* m_Renderer = nullptr;

	};

	Application* CreateApplication();

}

