// Copyright Levi Spevakow (C) 2024

#include "Renderer.h"

#include "Window.h"
#include "Log.h"
#include "sdl/SDL_vulkan.h"
#include <iostream>
#include <set>

Tempus::Renderer::Renderer()
{
}

Tempus::Renderer::~Renderer()
{
	Cleanup();
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

	if (!CreateVulkanInstance())
	{
		return false;
	}

	if (m_bEnableValidationLayers && !SetupDebugMessenger())
	{
		return false;
	}

	if (!CreateSurface(window))
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

	uint32_t instanceVersion = 0;

	if (vkEnumerateInstanceVersion(&instanceVersion) == VK_SUCCESS) 
	{
		TPS_CORE_INFO("Vulkan version: {0}.{1}.{2}", VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion));
	}

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

	// Logging of device information
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &deviceFeatures);

	std::cout << "Device Info:" << '\n';
	std::cout << '\t' << "Name: " << deviceProperties.deviceName << '\n';
	std::cout << '\t' << "ID: " << deviceProperties.deviceID << '\n';
	std::cout << '\t' << "Type: " << deviceProperties.deviceType << '\n';
	std::cout << '\t' << "Driver Version: " << deviceProperties.driverVersion << '\n';
	std::cout << '\t' << "API Version: " << deviceProperties.apiVersion << '\n';
	std::cout << '\t' << "Vendor ID: " << deviceProperties.vendorID << '\n';

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

	// Modern Vulkan makes no distinction between instance and device layers, but it is still good to set these values for compatibility
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
		// Checking if device supports graphics queue
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		// Check if device supports present queue
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

bool Tempus::Renderer::IsDeviceSuitable(VkPhysicalDevice device)
{
	// Check if physical device supports desired queue families
	QueueFamilyIndices indices = FindQueueFamilies(device);
	// Check if physical device supports desired extensions
	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	// If this optional variable has a value then the device supports graphics family queue
	return indices.IsComplete() && extensionsSupported;

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

	std::string enabledExtensions = "\nDesired instance extensions: \n";
	for (const char* extension : extensions)
	{
		enabledExtensions += '\t';
		enabledExtensions += extension;
		enabledExtensions += '\n';
	}
	TPS_CORE_INFO(enabledExtensions);
	
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

	std::cout << "Available instance extensions: \n";

	for (const auto& extension : enumExtensions)
		std::cout << '\t' << extension.extensionName << '\n';

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	std::cout << '\n' << "Available Layers: \n";

	for (const auto& layer : layers) {
		std::cout << '\t' << layer.layerName << '\n';
	}

	std::cout << std::endl;

}

void Tempus::Renderer::Cleanup()
{

	if (m_bEnableValidationLayers) 
	{
		DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);
	}

	vkDestroyDevice(m_Device, nullptr);
	vkDestroySurfaceKHR(m_VkInstance, m_VkSurface, nullptr);
	vkDestroyInstance(m_VkInstance, nullptr);
}
