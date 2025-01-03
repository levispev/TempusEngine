// Copyright Levi Spevakow (C) 2024

#pragma once

#include "Core.h"

#include "sdl/SDL.h"
#include <vector>
#include "vulkan/vulkan.h"
#include <optional>

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

		// Struct for potential queue families
		// Currently only searching for graphics queue family but will add more later
		struct QueueFamilyIndices
		{
			// Optional value to represent if queue family exists
			std::optional<uint32_t> graphicsFamily;

			bool IsComplete() 
			{
				return graphicsFamily.has_value();
			}
		};

		bool CreateVulkanInstance();
		bool SetupDebugMessenger();
		bool PickPhysicalDevice();
		bool CreateLogicalDevice();
		bool CreateSurface(class Window* window);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void LogExtensionsAndLayers();

		void Cleanup();

	private:

		// Will be replaced with proper Vulkan instantiation
		SDL_Renderer* m_Renderer = nullptr;

		VkInstance m_vkInstance = VK_NULL_HANDLE;
		VkSurfaceKHR m_vkSurface = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;

		// Standard validation layer
		const std::vector<const char*> m_validationLayers = 
		{
			DESIRED_VK_LAYER
		};

#ifdef TPS_DEBUG
		const bool m_bEnableValidationLayers = true;
#else
		const bool m_bEnableValidationLayers = false;
#endif

	};

}