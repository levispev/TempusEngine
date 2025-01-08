// Copyright Levi Spevakow (C) 2024

#include "Renderer.h"

#include "Window.h"
#include "Log.h"
#include "sdl/SDL_vulkan.h"
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm> 

Tempus::Renderer::Renderer()
{
}

Tempus::Renderer::~Renderer()
{
	Cleanup();
}

void Tempus::Renderer::Update()
{

}

bool Tempus::Renderer::Init(Tempus::Window* window)
{

	m_Window = window;

	if (!m_Window) 
	{
		return false;
	}

	if (!CreateVulkanInstance())
	{
		return false;
	}

	if (m_bEnableValidationLayers && !SetupDebugMessenger())
	{
		return false;
	}

	if (!CreateSurface(m_Window))
	{
		return false;
	}

	if (!PickPhysicalDevice())
	{
		return false;
	}

	if (!CreateLogicalDevice()) 
	{
		return false;
	}

	if (!CreateSwapChain()) 
	{
		return false;
	}

	return true;

}

int Tempus::Renderer::RenderClear()
{
	return 0;
	//return SDL_RenderClear(m_Renderer);
}

void Tempus::Renderer::RenderPresent()
{
	//SDL_RenderPresent(m_Renderer);
}

int Tempus::Renderer::SetRenderDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return 0;
	//return SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
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
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Retrieving minimum platform specific required extensions for SDL surface
	auto extensions = GetRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();


	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (m_bEnableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

		// This must be set in order to have debug callback support for the creation and destruction of instance
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	}
	else 
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Creating instance
	if (vkCreateInstance(&createInfo, nullptr, &m_VkInstance) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create Vulkan instance!");
		return false;
	}

	LogExtensionsAndLayers();

	return true;

}

bool Tempus::Renderer::SetupDebugMessenger()
{

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) 
	{
    	TPS_CORE_CRITICAL("Failed to set up debug messenger!");
		return false;
	}		

    return true;
}

bool Tempus::Renderer::PickPhysicalDevice()
{
	// Get device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);

	if (deviceCount == 0) 
	{
		TPS_CORE_CRITICAL("Failed to find GPU with Vulkan support!");
		return false;
	}

	// Get physical devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

	// Check if device is suitable
	for (const auto& device : devices) 
	{
		if (IsDeviceSuitable(device)) 
		{
			m_PhysicalDevice = device;
			break;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE) 
	{
		TPS_CORE_CRITICAL("Failed to find suitable GPU!");
		return false;
	}

	LogDeviceInfo(m_PhysicalDevice);

	return true;
}

bool Tempus::Renderer::CreateLogicalDevice()
{

	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	// Set of all unique queue families
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) 
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		// Currently only creating 1 queue, however the priority must still be specified
		queueCreateInfo.pQueuePriorities = &queuePriority;
		// Pushing into list of queue create infos
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

	// Modern Vulkan makes no distinction between instance and device layers and therefore ignores this data,
	// however it is still good to set these values for compatibility
	if (m_bEnableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
	} 
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create logical device!");
    	return false;
	}

	// Retrieve reference to devices graphics queue, index 0 because we only have 1 queue
	vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
	
	return true;
}

bool Tempus::Renderer::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	// Amount of images in the swapchain (min + 1)
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// Ensure value doesn't exceed maximum. maxImageCount = 0 denotes there is no maximum
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
	{
    	imageCount = swapChainSupport.capabilities.maxImageCount;
	}


	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_VkSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	// Specifies amount of layers each image consists of. Always 1 for traditional rendering
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	// No transform modifications (eg. horizonal flip)
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	// Ignoring alpha channel
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	// Ignore the pixels that are occluded. Best performance.
	// Would probably need to disable this if doing some sort of screen space rendering
	createInfo.clipped = VK_TRUE;
	// When a swap chain is invalidated or destroyed and a new one is created, the old one must be provided
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	// If our graphics queue and presentation queue reside in different queue families, we must specify
	// how the swap chain will handle these images
	if (indices.graphicsFamily != indices.presentFamily) 
	{
		// Images can be shared between queue families without explicit ownership transferrence
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} 
	else 
	{
		// Explicit ownership by a queue family, best performance
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create swap chain!");
		return false;
	}


	// Querying for swap chain image count. Only minimum was specified in creation info, actual number may be higher
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());
	
	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtent = extent;

	return true;
}

bool Tempus::Renderer::CreateSurface(Tempus::Window* window)
{

	if (!window || !window->GetNativeWindow()) 
	{
		return false;
	}

	return SDL_Vulkan_CreateSurface(window->GetNativeWindow(), m_VkInstance, &m_VkSurface);

}

Tempus::Renderer::QueueFamilyIndices Tempus::Renderer::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Retrieve queue family count
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	// Retrieve queue families
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		// Checking if queue family supports graphics queue
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		// Check if queue family supports present queue
		// These capabilities may reside in the same queue family
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_VkSurface, &presentSupport);

		if (presentSupport) 
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete()) 
		{
			break;
		}

		i++;
	}

	return indices;
}

Tempus::Renderer::SwapChainSupportDetails Tempus::Renderer::QuerySwapChainSupport(VkPhysicalDevice device)
{

	SwapChainSupportDetails details;

	// Query surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_VkSurface, &details.capabilities);

	// Query supported formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_VkSurface, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_VkSurface, &formatCount, details.formats.data());
	}

	// Query supported presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_VkSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_VkSurface, &presentModeCount, details.presentModes.data());
	}

    return details;
}

VkPresentModeKHR Tempus::Renderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) 
	{
		// Mailbox is desired if available
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
            return availablePresentMode;
        }
    }

	// This present mode is guaranteed to exist
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Tempus::Renderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	for (const auto& availableFormat : availableFormats) 
	{
		// BGRA 32 bits per pixel
		// SRGB
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	}

    return availableFormats[0];
}

VkExtent2D Tempus::Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	// Special value check
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) 
	{
		return capabilities.currentExtent;
	} 
	else 
	{
		int width, height;
		// The drawable size (pixels) may differ from the window size (screen coordinates) on high DPI displays (Mac Retina)
		// Vulkan wants exact pixel size, not screen coordinates
		SDL_Vulkan_GetDrawableSize(m_Window->GetNativeWindow(), &width, &height);

		VkExtent2D actualExtent = 
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;	
	}
}

bool Tempus::Renderer::IsDeviceSuitable(VkPhysicalDevice device)
{
	// Check if physical device supports desired queue families
	QueueFamilyIndices indices = FindQueueFamilies(device);
	// Check if physical device supports desired extensions
	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	// Check if devices swap chain has adequate support
	bool swapChainAdequate = false;
	// Only query if extension support exists
	if (extensionsSupported) 
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() && extensionsSupported && swapChainAdequate;

}

bool Tempus::Renderer::CheckValidationLayerSupport()
{
	// Retrieve available layer count
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Enumerate available layers
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Check if enumerated validation layers contain desired layer
	for (const char* layerName : m_ValidationLayers) 
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

bool Tempus::Renderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

	for (const auto& extension : availableExtensions) 
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

std::vector<const char*> Tempus::Renderer::GetRequiredExtensions()
{
	uint32_t extensionCount = 0;

	// Get extension count
	SDL_Vulkan_GetInstanceExtensions(nullptr, &extensionCount, nullptr);

	std::vector<const char*> extensions(extensionCount);

	// Get minimum required extensions
	SDL_Vulkan_GetInstanceExtensions(nullptr, &extensionCount, extensions.data());

	if (m_bEnableValidationLayers) 
	{
		// Adding debug extension if validation layers are enabled
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	
	return extensions;
}

void Tempus::Renderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	// Can be used to pass application pointer
	createInfo.pUserData = nullptr;
}

void Tempus::Renderer::LogExtensionsAndLayers()
{

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> enumExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, enumExtensions.data());

	auto enabledExtensions = GetRequiredExtensions();

	std::stringstream ss;

	ss << "\nInstance extensions: \n";

	for (const auto& extension : enumExtensions)
	{
		bool isEnabled = false;

		if (std::find_if(enabledExtensions.begin(), enabledExtensions.end(), 
			[extension](const char* str) 
			{ 
				return std::strcmp(str, extension.extensionName) == 0; 
			}
			) != enabledExtensions.end())
		{
			isEnabled = true;
		}

		ss << (isEnabled ? (COLOR_YELLOW + std::string("[ACTIVE]") + COLOR_GREEN) : "\t") << extension.extensionName << COLOR_RESET << '\n';
	}

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	ss << '\n' << "Validation Layers: \n";

	for (const auto& layer : layers) 
	{
		bool isEnabled = false;

		if(m_bEnableValidationLayers)
		{
			if (std::find_if(m_ValidationLayers.begin(), m_ValidationLayers.end(), 
				[layer](const char* str) 
				{ 
					return std::strcmp(str, layer.layerName) == 0; 
				}
				) != m_ValidationLayers.end())
			{
				isEnabled = true;
			}
		}

		ss << (isEnabled ? (COLOR_YELLOW + std::string("[ACTIVE]") + COLOR_GREEN) : "\t") << layer.layerName << COLOR_RESET << '\n';
	}

	TPS_CORE_INFO(ss.str());

}

void Tempus::Renderer::LogDeviceInfo(VkPhysicalDevice device)
{
	// Logging of device information
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	std::stringstream ss;

	ss << "\nDevice Info:" << '\n';
	ss << '\t' << "Name: " << deviceProperties.deviceName << '\n';
	ss << '\t' << "ID: " << deviceProperties.deviceID << '\n';
	ss << '\t' << "Type: " << deviceProperties.deviceType << '\n';
	ss << '\t' << "Driver Version: " << deviceProperties.driverVersion << '\n';
	ss << '\t' << "API Version: " << deviceProperties.apiVersion << '\n';
	ss << '\t' << "Vendor ID: " << deviceProperties.vendorID << '\n';

	TPS_CORE_INFO(ss.str());
}

void Tempus::Renderer::Cleanup()
{

	if (m_bEnableValidationLayers) 
	{
		DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
	vkDestroyDevice(m_Device, nullptr);
	vkDestroySurfaceKHR(m_VkInstance, m_VkSurface, nullptr);
	vkDestroyInstance(m_VkInstance, nullptr);
}
