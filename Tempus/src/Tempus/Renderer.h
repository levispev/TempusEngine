// Copyright Levi Spevakow (C) 2024

#pragma once

#include "Core.h"

#include "sdl/SDL.h"
#include <vector>

namespace Tempus {

	class TEMPUS_API Renderer {


	public:

		Renderer();
		~Renderer();

		void Update();

		bool Init(class Window* window);

		int RenderClear();
		void RenderPresent();
		int SetRenderDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	private:

		bool CreateVulkanInstance();
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();

	private:

		SDL_Renderer* m_Renderer = nullptr;

		// Standard validation layer
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

#ifdef TPS_DEBUG
		const bool m_bEnableValidationLayers = false;
#else
		const bool m_bEnableValidationLayers = true;
#endif

	};

}