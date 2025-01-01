// Copyright Levi Spevakow (C) 2024

#include "Renderer.h"

#include "Window.h"
#include "vulkan/vulkan.h"
#include "Log.h"
#include "sdl/SDL_vulkan.h"

Tempus::Renderer::Renderer()
{
}

Tempus::Renderer::~Renderer()
{
}

void Tempus::Renderer::Update()
{
	SDL_RenderClear(m_Renderer);
	SDL_RenderPresent(m_Renderer);
}

bool Tempus::Renderer::Init(Tempus::Window* window)
{

	if (!window) 
	{
		return false;
	}

	m_Renderer = SDL_CreateRenderer(window->GetNativeWindow(), -1, 0);
	SDL_SetRenderDrawColor(m_Renderer, 19, 61, 102, 255);

	CreateVulkanInstance();

	return m_Renderer;

}

int Tempus::Renderer::RenderClear()
{
	return SDL_RenderClear(m_Renderer);
}

void Tempus::Renderer::RenderPresent()
{
	SDL_RenderPresent(m_Renderer);
}

int Tempus::Renderer::SetRenderDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
}

bool Tempus::Renderer::CreateVulkanInstance()
{
	if (m_bEnableValidationLayers && !CheckValidationLayerSupport())
	{
		TPS_CORE_CRITICAL("Validation layers requested, but not available!");
		return false;
	}

	// Application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	// @TODO In the future this application specific data will be read from a config file
	appInfo.pApplicationName = "Sandbox";
	appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	appInfo.pEngineName = "Tempus Engine";
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = GetRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (m_bEnableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	return true;

}

bool Tempus::Renderer::CheckValidationLayerSupport()
{
	// Retrieve available layer count
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Enumerate available layers
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Check if enumerated validation layers contain desired layer ("VK_LAYER_KHRONOS_validation")
	for (const char* layerName : validationLayers) 
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) 
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) 
		{
			return false;
		}

	}

	return true;
}

std::vector<const char*> Tempus::Renderer::GetRequiredExtensions()
{
	uint32_t extensionCount = 0;

	// Get extension count
	SDL_Vulkan_GetInstanceExtensions(nullptr, &extensionCount, nullptr);

	std::vector<const char*> extensions(extensionCount);

	// Get extensions
	SDL_Vulkan_GetInstanceExtensions(nullptr, &extensionCount, extensions.data());

	if (m_bEnableValidationLayers) 
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
