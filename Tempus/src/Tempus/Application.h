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

		VkInstance m_Instance = nullptr;

		SDL_Window* m_Window = nullptr;

	};

}

