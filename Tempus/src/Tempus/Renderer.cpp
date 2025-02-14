// Copyright Levi Spevakow (C) 2025

#include "Renderer.h"

#include "Window.h"
#include "Log.h"
#include "Utils/FileUtils.h"
#include "sdl/SDL_vulkan.h"
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm> 
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_vulkan.h"
#include <vulkan/vk_enum_string_helper.h>


Tempus::Renderer::Renderer()
{
}

Tempus::Renderer::~Renderer()
{
	Cleanup();
}

void Tempus::Renderer::Update()
{
	DrawImGui();
	DrawFrame();
}

bool Tempus::Renderer::Init(Tempus::Window* window)
{

	m_Window = window;

	if (!m_Window) 
	{
		return false;
	}

	CreateVulkanInstance();

	if (m_bEnableValidationLayers)
	{
		SetupDebugMessenger();
	}

	CreateSurface(m_Window);
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateCommandPool();
	CreateCommandBuffer();
	CreateSyncObjects();

	InitImGui();

	return true;

}

void Tempus::Renderer::SetClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	m_ClearColor[0] = r / 255.0f;
	m_ClearColor[1] = g / 255.0f;
	m_ClearColor[2] = b / 255.0f;
	m_ClearColor[3] = a / 255.0f;
}

void Tempus::Renderer::DrawFrame()
{
	// Wait for previous frame to finish drawing
	vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	// Retrieve image from swap chain
	VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

	// Check if swapchain has been invalidated
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		TPS_CORE_CRITICAL("Failed to acquire swap chain image!");
	}

	// Reset fence signal
	vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);

	RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);


	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame]};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
	{
		m_FramebufferResized = false;
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to present swap chain image!");
	}

	m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

}

void Tempus::Renderer::DrawImGui()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Application Stats");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Swapchain extent: %ux%u", m_SwapChainExtent.width, m_SwapChainExtent.height);
	ImGui::End();

	ImGui::Begin("Device Info");
		ImGui::Text("Name: %s", m_DeviceDetails.name.c_str());
		ImGui::Text("Type: %s", m_DeviceDetails.type.c_str());
		ImGui::Text("ID: %u", m_DeviceDetails.id);
		ImGui::Text("Driver Version: %u.%u.%u", VK_VERSION_MAJOR(m_DeviceDetails.driverVersion), VK_VERSION_MINOR(m_DeviceDetails.driverVersion), VK_VERSION_PATCH(m_DeviceDetails.driverVersion));
		ImGui::Text("API Version: %u.%u.%u", VK_VERSION_MAJOR(m_DeviceDetails.apiVersion), VK_VERSION_MINOR(m_DeviceDetails.apiVersion), VK_VERSION_PATCH(m_DeviceDetails.apiVersion));
		ImGui::Text("Vendor ID: %u", m_DeviceDetails.vendorId);
	ImGui::End();

	ImGui::Render();
}

void Tempus::Renderer::CreateVulkanInstance()
{
	if (m_bEnableValidationLayers && !CheckValidationLayerSupport())
	{
		TPS_CORE_CRITICAL("Validation layers requested, but not available!");
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
	std::vector<const char*> extensions = GetRequiredExtensions();

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
	}

	LogExtensionsAndLayers();

}

void Tempus::Renderer::SetupDebugMessenger()
{

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) 
	{
    	TPS_CORE_CRITICAL("Failed to set up debug messenger!");
	}		

}

void Tempus::Renderer::PickPhysicalDevice()
{
	// Get device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);

	if (deviceCount == 0) 
	{
		TPS_CORE_CRITICAL("Failed to find GPU with Vulkan support!");
	}
	
	// Get physical devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

	uint32_t highestScore = 0;

	// Check if device is suitable
	for (const auto& device : devices) 
	{
		uint32_t score = GetDeviceScore(device);

		if (IsDeviceSuitable(device) && score > highestScore) 
		{
			highestScore = score;
			m_PhysicalDevice = device;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE) 
	{
		TPS_CORE_CRITICAL("Failed to find suitable GPU!");
	}

	LogDeviceInfo();

}

void Tempus::Renderer::CreateLogicalDevice()
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
	}

	// Retrieve reference to devices graphics queue, index 0 because we only have 1 queue
	vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
	
}

void Tempus::Renderer::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

	LogSwapchainDetails(swapChainSupport);

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
	}

	std::stringstream ss;

	ss << "\nCreated Swapchain details: \n";
	ss << "\tFormat: " << string_VkFormat(surfaceFormat.format) << '\n';
	ss << "\tColor Space: " << string_VkColorSpaceKHR(surfaceFormat.colorSpace) << '\n';
	ss << "\tPresent mode: " << string_VkPresentModeKHR(presentMode) << '\n';
	ss << "\tExtent: " << extent.width << "x" << extent.height << '\n';
	ss << "\tImage count: " << imageCount << '\t';

	TPS_CORE_INFO(ss.str());

	// Querying for swap chain image count. Only minimum was specified in creation info, actual number may be higher
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
	// These images are created by the swapchain and do not need to be cleaned up
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());
	
	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtent = extent;

}

void Tempus::Renderer::CreateImageViews()
{

	m_SwapChainImageViews.resize(m_SwapChainImages.size());

	for (size_t i = 0; i < m_SwapChainImages.size(); i++) 
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_SwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_SwapChainImageFormat;
		
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) 
		{
			TPS_CORE_CRITICAL("Failed to create image view!");
		}

	}

}

void Tempus::Renderer::CreateRenderPass()
{
	// Single colour attachment
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapChainImageFormat;
	// 1 sample because no multi sampling right now
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// Currently not using stencil
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// Final layout for presentation to swapchain
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create render pass!");
	}

}

void Tempus::Renderer::CreateGraphicsPipeline()
{

	auto vertShaderCode = FileUtils::ReadFile("bin/shaders/vert.spv");
	auto fragShaderCode = FileUtils::ReadFile("bin/shaders/frag.spv");

	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	// Currently hard coding vertex data in shader so setting these to empty
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional


	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	// Indicates triangle from every 3 vertices without reuse
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Dynamic states can be modified at drawtime without having to recreate the entire pipeline.
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;


	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// If this value is set to VK_TRUE, then fragments outside of near and far plane are clamped instead of discarded.
	rasterizer.depthClampEnable = VK_FALSE;
	// If this value is set to VK_TRUE then the rasterizers output is effectively disabled
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// Multi sampling disabled for now
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; 
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; 
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; 
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; 
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; 
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional


	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create pipeline layout!");
	}

	// Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = m_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
	vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);

}

void Tempus::Renderer::CreateFrameBuffers()
{
	m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

	for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) 
	{

		VkImageView attachments[] = 
		{
			m_SwapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_SwapChainExtent.width;
		framebufferInfo.height = m_SwapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) 
		{
			TPS_CORE_CRITICAL("Failed to create framebuffer!");
		}
	}

}

void Tempus::Renderer::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create command pool!");
	}

}

void Tempus::Renderer::CreateCommandBuffer()
{

	m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

	if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to allocate command buffers!");
	}

}

void Tempus::Renderer::CreateSyncObjects()
{
	m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// Setting fence to be signalled on creation for first call of DrawFrame()
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
		{
			TPS_CORE_CRITICAL("Failed to create semaphores!");
		}
	}

}

void Tempus::Renderer::RecreateSwapChain()
{
	TPS_CORE_INFO("Recreating swapchain!");
	vkDeviceWaitIdle(m_Device);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateFrameBuffers();
}

void Tempus::Renderer::InitImGui()
{
	// Huge buffer. Same as in example code. Can reduce
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 500 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 500 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 500 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 500 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 500 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 500 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 500 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 500 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 500 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 500 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 500 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = std::size(pool_sizes);
	poolInfo.pPoolSizes = pool_sizes;
	
	if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_ImguiPool) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create imgui descriptor pool!");
	}

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	if (!m_Window || !m_Window->GetNativeWindow()) 
	{
		TPS_CORE_CRITICAL("Invalid window!");
	}

	ImGui_ImplSDL2_InitForVulkan(m_Window->GetNativeWindow());

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = m_VkInstance;
	initInfo.PhysicalDevice = m_PhysicalDevice;
	initInfo.Device = m_Device;
	initInfo.Queue = m_GraphicsQueue;
	initInfo.DescriptorPool = m_ImguiPool;
	initInfo.MinImageCount = 3;
	initInfo.ImageCount = 3;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.RenderPass = m_RenderPass;

	ImGui_ImplVulkan_Init(&initInfo);
	//ImGui_ImplVulkan_CreateFontsTexture();

}

void Tempus::Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_RenderPass;
	renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_SwapChainExtent;
	VkClearValue clearColor = { {{m_ClearColor[0],m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]}}};
	//TPS_CORE_INFO("Clear Color: {}, {}, {}", m_ClearColor[0],m_ClearColor[1], m_ClearColor[2]);
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	// Begin render pass
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	// Bind to pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

	// Viewport and scissor are dynamic values in our pipeline and therefore must be set in command buffer before issuing draw command
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapChainExtent.width);
	viewport.height = static_cast<float>(m_SwapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to record command buffer!");
	}

}

VkShaderModule Tempus::Renderer::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	
	if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create shader module!");
	}

	return shaderModule;
}

void Tempus::Renderer::CreateSurface(Tempus::Window* window)
{
	if(!window || !SDL_Vulkan_CreateSurface(window->GetNativeWindow(), m_VkInstance, &m_VkSurface))
	{
		TPS_CORE_CRITICAL("Failed to create surface!");
	}
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

uint32_t Tempus::Renderer::GetDeviceScore(VkPhysicalDevice device)
{

	uint32_t score = 0;

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	switch (deviceProperties.deviceType) 
	{
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		score += 1;
		break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		score += 2;
		break;
	default:
		break;
	}

	return score;
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

void Tempus::Renderer::LogDeviceInfo()
{

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

	std::stringstream ss;
	ss << "\nFound Devices:" << '\n';
	uint32_t i = 0;

	for (const VkPhysicalDevice& device : devices) 
	{
		// Logging of device information
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		DeviceDetails deviceDetails;
		deviceDetails.name = deviceProperties.deviceName;
		deviceDetails.id = deviceProperties.deviceID;
		deviceDetails.type = std::string(string_VkPhysicalDeviceType(deviceProperties.deviceType)).substr(24);
		deviceDetails.driverVersion = deviceProperties.driverVersion;
		deviceDetails.apiVersion = deviceProperties.apiVersion;
		deviceDetails.vendorId = deviceProperties.vendorID;

		if (device == m_PhysicalDevice) 
		{
			ss << COLOR_YELLOW << "[ACTIVE]" << COLOR_RESET;
			m_DeviceDetails = deviceDetails;
		}
		ss << "Device #" << i++ << '\n';
		ss << '\t' << "Name: " << deviceDetails.name << '\n';
		ss << '\t' << "ID: " << deviceDetails.id << '\n';
		ss << '\t' << "Type: " << deviceDetails.type << '\n';
		ss << '\t' << "Driver Version: " << VK_VERSION_MAJOR(deviceDetails.driverVersion) << '.' << VK_VERSION_MINOR(deviceDetails.driverVersion) << '.' << VK_VERSION_PATCH(deviceDetails.driverVersion) <<'\n';
		ss << '\t' << "API Version: " << VK_VERSION_MAJOR(deviceDetails.apiVersion) << '.' << VK_VERSION_MINOR(deviceDetails.apiVersion) << '.' << VK_VERSION_PATCH(deviceDetails.apiVersion) << '\n';
		ss << '\t' << "Vendor ID: " << deviceDetails.vendorId << '\n';
		ss << '\n';

		// Logging of queue support
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilies.size(); i++) 
		{
			const auto& queueFamily = queueFamilies[i];

			ss << "Queue Family #" << i << '\n';
			ss << "\tMax queue count: " << queueFamily.queueCount << '\n';
			ss << "\tFlags: ";

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) ss << "Graphics ";
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) ss << "Compute ";
			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) ss << "Transfer ";
			if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ss << "Sparse Binding ";
			if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) ss << "Protected ";
			if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) ss << "Video Decode ";
			//if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) std::cout << "Video Encode ";

			ss << '\n';

			ss << "\tMinimum Image Transfer Granularity: "
				<< queueFamily.minImageTransferGranularity.width << "x"
				<< queueFamily.minImageTransferGranularity.height << "x"
				<< queueFamily.minImageTransferGranularity.depth << "\n";
		}

		ss << '\n';

	}

	TPS_CORE_INFO(ss.str());

}

void Tempus::Renderer::LogSwapchainDetails(const SwapChainSupportDetails &details)
{

	std::stringstream ss;

	ss << "\nSwapChain support details:\n";

	ss << "\tMin Image Count: " << details.capabilities.minImageCount << '\n';
	ss << "\tMax Image Count: " << details.capabilities.maxImageCount << '\n';
	ss << "\tMin Image Extent: " << details.capabilities.minImageExtent.width << 'x' << details.capabilities.minImageExtent.height << '\n';
	ss << "\tMax Image Extent: " << details.capabilities.maxImageExtent.width << 'x' << details.capabilities.maxImageExtent.height << '\n';
	
	ss << "Formats: \n";
	
	for(VkSurfaceFormatKHR surfaceFormat : details.formats)
	{
		ss << "\t" << string_VkFormat(surfaceFormat.format) << '\n';
	}

	ss << "Available Color spaces: \n";

	for(VkSurfaceFormatKHR surfaceFormat : details.formats)
	{
		ss << "\t" << string_VkColorSpaceKHR(surfaceFormat.colorSpace) << '\n';
	}

	ss << "Available Present modes: \n";

	for(VkPresentModeKHR presentMode : details.presentModes)
	{
		ss << '\t' << string_VkPresentModeKHR(presentMode) << '\n';
	}

	TPS_CORE_INFO(ss.str());

}

void Tempus::Renderer::CleanupSwapChain()
{
	for (auto framebuffer : m_SwapChainFramebuffers)
	{
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	}

	for (auto imageView : m_SwapChainImageViews)
	{
		vkDestroyImageView(m_Device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
}

void Tempus::Renderer::Cleanup()
{

	// Wait for all async objects to finish
	vkDeviceWaitIdle(m_Device);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(m_Device, m_ImguiPool, nullptr);

	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
	}
	
	CleanupSwapChain();

	vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
	vkDestroyDevice(m_Device, nullptr);

	if (m_bEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(m_VkInstance, m_VkSurface, nullptr);
	vkDestroyInstance(m_VkInstance, nullptr);

}
