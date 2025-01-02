// Copyright Levi Spevakow (C) 2024

#pragma once

#include "Core.h"

#include "sdl/SDL.h"
#include <vector>
#include "vulkan/vulkan.h"

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
		bool PickPhysicalDevice();
		bool CreateSurface(class Window* window);
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void LogExtensionsAndLayers();

	private:

		// Will be replaced with proper Vulkan instantiation
		SDL_Renderer* m_Renderer = nullptr;
		VkInstance m_vkInstance;
		VkSurfaceKHR m_vkSurface;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

		// Standard validation layer
		const std::vector<const char*> m_validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

#ifdef TPS_DEBUG
		const bool m_bEnableValidationLayers = true;
#else
		const bool m_bEnableValidationLayers = false;
#endif

	};

}