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
#include "sdl/SDL.h"
#define GLM_FORCE_RADIANS
// Vulkan uses 0 - 1 depth range. Default is 1 - 0 (OpenGl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <chrono>

#include "Application.h"
#include "Scene.h"
#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/SceneManager.h"
#include "Events/EventDispatcher.h"
#include "stb_image/stb_image.h"

#include "Entity/Entity.h"
#include "Utils/Time.h"

#define NVIDIA_VENDOR_ID 0X10DE

Tempus::Renderer::Renderer()
{
	//stbi_set_flip_vertically_on_load(true);
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
	LogSwapchainDetails(QuerySwapChainSupport(m_PhysicalDevice));
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateDepthResources();
	CreateFrameBuffers();
	CreateCommandPool();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
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

void Tempus::Renderer::OnEvent(const SDL_Event& event)
{
	if (event.type == SDL_WINDOWEVENT) 
	{
		if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) 
		{
			TPS_CORE_WARN("Window resized! {}x{}", event.window.data1, event.window.data2);
		}
	}
	else if (event.type == SDL_KEYDOWN) // Temporary input handling 
	{
		char key = (char)event.key.keysym.sym;
		//TPS_CORE_TRACE("DOWN: {0}", (int)event.key.keysym.scancode);
		if(m_InputMap.contains(event.key.keysym.scancode))
		{
			m_InputBits.set(m_InputMap[event.key.keysym.scancode]);	
		}
	}
	else if (event.type == SDL_KEYUP)
	{
		char key = (char)event.key.keysym.sym;
		//TPS_CORE_TRACE("UP: {0}", (int)event.key.keysym.scancode);
		if(m_InputMap.contains(event.key.keysym.scancode))
		{
			m_InputBits.reset(m_InputMap[event.key.keysym.scancode]);	
		}
	}
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
		TPS_CORE_WARN("Swapchain out of date!");
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

	UpdateUniformBuffer(m_CurrentFrame);

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
		TPS_CORE_WARN("Swapchain out of date!");
		m_FramebufferResized = false;
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to present swap chain image!");
	}

	m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Tempus::Renderer::UpdateUniformBuffer(uint32_t currentImage)
{
	Scene* activeScene = SCENE_MANAGER->GetActiveScene();

	if (!activeScene)
	{
		return;
	}

	CameraComponent* editorCam = activeScene->GetComponent<CameraComponent>(0);
	TransformComponent* editorCamTransform = activeScene->GetComponent<TransformComponent>(0);

	if (editorCam && editorCamTransform)
	{
		editorCamTransform->Position.x += m_InputBits.test(0) * (Time::GetDeltaTime() * 10.0f);
		editorCamTransform->Position.y += m_InputBits.test(1) * (Time::GetDeltaTime() * 10.0f);
		editorCamTransform->Position.x -= m_InputBits.test(2) * (Time::GetDeltaTime() * 10.0f);
		editorCamTransform->Position.y -= m_InputBits.test(3) * (Time::GetDeltaTime() * 10.0f);

		editorCamTransform->Position.z -= m_InputBits.test(4) * (Time::GetDeltaTime() * 10.0f);
		editorCamTransform->Position.z += m_InputBits.test(5) * (Time::GetDeltaTime() * 10.0f);

		m_EditorCamPos = editorCamTransform->Position;
		m_EditorCamForward = editorCamTransform->GetForwardVector();
	}
	else
	{
		m_EditorCamPos = glm::vec3(-3.0f, 0.0f, 0.0f);
		m_EditorCamForward = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	
	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), Time::GetTime() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.model = glm::translate(ubo.model, glm::vec3(0.0f, 0.0f, glm::sin(Time::GetTime())));
	ubo.view = glm::lookAt(m_EditorCamPos, m_EditorCamPos + m_EditorCamForward, glm::vec3(0.0f, 0.0f, 1.0f));

	switch (editorCam->ProjectionType)
	{
		case CamProjectionType::Perspective:
			ubo.proj = glm::perspective(glm::radians(editorCam->Fov), editorCam->AspectRatio, editorCam->NearClip, editorCam->FarClip);
		break;
		case CamProjectionType::Orthographic:
			// Ortho size is the total height
			float halfHeight = editorCam->OrthoSize * 0.5f;
			float halfWidth  = halfHeight * editorCam->AspectRatio;

			float left   = -halfWidth;
			float right  =  halfWidth;
			float bottom = -halfHeight;
			float top    =  halfHeight;

			// Preferred for Vulkan. right-handed, zero-to-one depth
			ubo.proj = glm::orthoRH_ZO(left, right, bottom, top, editorCam->NearClip, editorCam->FarClip);
		break;
	}

	// Accounting for inverted Y coordinate between OpenGL and Vulkan
	ubo.proj[1][1] *= -1;

	memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Tempus::Renderer::DrawImGui()
{
	static bool showAboutPopup = false;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	// -- Main menu bar
	if (ImGui::BeginMainMenuBar()) 
	{
		if (ImGui::BeginMenu("File")) 
		{
			if (ImGui::MenuItem("New"))  {  }
			if (ImGui::MenuItem("Open")) {  }
			if (ImGui::MenuItem("Save")) {  }
			if (ImGui::MenuItem("Exit")) { Application::RequestExit(); }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) 
		{
			if (ImGui::MenuItem("Undo")) {  }
			if (ImGui::MenuItem("Redo")) { }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) 
		{
			if (ImGui::MenuItem("Fullscreen")) {  }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")) 
		{
			if (ImGui::MenuItem("About")) { showAboutPopup = true;}
			if (ImGui::MenuItem("Force Crash")) { TPS_CORE_CRITICAL("Force game crash!"); }
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// -- Menu bar "About" popup
	if (showAboutPopup) 
	{
		ImGui::OpenPopup("About"); 
		if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) 
		{
			ImGui::Text("Tempus Engine v0.0.1");
			ImGui::Text("Built with SDL2 and ImGui");
			ImGui::Text("Created by Levi Spevakow");
			ImGui::Separator();

			// Center the Close button
			// Button width including padding
			float buttonWidth = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2; 
			// Usable window width
			float windowWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
			float offset = (windowWidth - buttonWidth) * 0.5f; 
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset); 

			if (ImGui::Button("Close")) 
			{
				showAboutPopup = false; 
				ImGui::CloseCurrentPopup(); 
			}
			ImGui::EndPopup();
		}
	}

	// -- Application stats
	ImGui::Begin("Application Stats");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Swapchain extent: %ux%u", m_SwapChainExtent.width, m_SwapChainExtent.height);
		ImGui::Text("Delta Time: %f", Time::GetDeltaTime());
		ImGui::Text("Time: %f", Time::GetTime());
	ImGui::End();

	// -- Device info
	ImGui::Begin("Device Info");
		ImGui::Text("Name: %s", m_DeviceDetails.name.c_str());
		ImGui::Text("Type: %s", m_DeviceDetails.type.c_str());
		ImGui::Text("ID: %u", m_DeviceDetails.id);
		// Nvidia vendor ID
		if (m_DeviceDetails.vendorId == NVIDIA_VENDOR_ID)
		{
			ImGui::Text("Driver Version: %u.%u.%u", ((m_DeviceDetails.driverVersion >> 22) & 0x3FF), ((m_DeviceDetails.driverVersion >> 14) & 0xFF), ((m_DeviceDetails.driverVersion >> 6) & 0xFF));
		}
		else
		{
			ImGui::Text("Driver Version: %u.%u.%u", VK_VERSION_MAJOR(m_DeviceDetails.driverVersion), VK_VERSION_MINOR(m_DeviceDetails.driverVersion), VK_VERSION_PATCH(m_DeviceDetails.driverVersion));
		}
		ImGui::Text("API Version: %u.%u.%u", VK_VERSION_MAJOR(m_DeviceDetails.apiVersion), VK_VERSION_MINOR(m_DeviceDetails.apiVersion), VK_VERSION_PATCH(m_DeviceDetails.apiVersion));
		ImGui::Text("Vendor ID: %u", m_DeviceDetails.vendorId);
	ImGui::End();

	// -- Clear color
	ImGui::Begin("Clear Color");
	ImGui::ColorPicker3("Color", &m_ClearColor[0]);
	ImGui::End();

	// -- Input visualization
	ImGui::Begin("Input");
	ImGui::Text("Input: %s", m_InputBits.to_string().c_str());
	ImGui::End();

	// -- Demo window for testing
	ImGui::ShowDemoWindow();

	// ImGui::Begin("Event Dispatcher");
	// 	ImGui::Text("Subscriber count: %u", EVENT_DISPATCHER->GetSubscriberCount());
	// ImGui::End();

	Scene* currentScene = SCENE_MANAGER->GetActiveScene();

	if (currentScene)
	{
		// --- Scene info
		ImGui::Begin("Scene Info");
			ImGui::Text("Name: %s", currentScene->GetName().c_str());
			ImGui::Text("Entity count: %u", currentScene->GetEntityCount());
			if(ImGui::Button("Add Entity"))
			{
				currentScene->AddEntity("Debug Entity");
			}
			static int id = 0;
			ImGui::InputInt("Selected Entity", &id);
			if(ImGui::Button("Remove Entity"))
			{
				currentScene->RemoveEntity(id);
			}

			static TPS_Private::ComponentRegistry::ComponentTypeInfo selectedComponent;
			if (ImGui::BeginCombo("Components", selectedComponent.name.c_str()))
			{
				const std::vector<TPS_Private::ComponentRegistry::ComponentTypeInfo>& registeredComponents = TPS_Private::ComponentRegistry::GetRegisteredComponents();
				for (const auto& component : registeredComponents)
				{
					if (ImGui::Selectable(component.name.c_str()))
					{
						selectedComponent = component;
					}
				}
				ImGui::EndCombo();
			}
			if(ImGui::Button("Add Component to entity"))
			{
				if (selectedComponent.defaultConstructor)
				{
					selectedComponent.defaultConstructor(currentScene, id);
				}
				else
				{
					TPS_CORE_ERROR("Cannot add component, no valid component selected!");	
				}
			}
		ImGui::End();

		// --- Scene outliner
		ImGui::Begin("Scene Outliner");
			{
				TPS_CALL_ONCE(ImGui::SetNextItemOpen, true);
				if (ImGui::TreeNode("Entities"))
				{
					std::vector<uint32_t> entIDs = currentScene->GetEntityIDs();
					for (const uint32_t entID : entIDs)
					{
						if (currentScene->GetComponentCount(entID))
						{
							TPS_CALL_ONCE(ImGui::SetNextItemOpen, true);
							if (ImGui::TreeNode(std::format("[{}] {}", entID, currentScene->GetEntityName(entID)).c_str()))
							{
								if (TransformComponent* transformComp = currentScene->GetComponent<TransformComponent>(entID))
								{
									TPS_CALL_ONCE(ImGui::SetNextItemOpen, true);
									if (ImGui::TreeNode(TempusUtils::GetClassDebugName<TransformComponent>()))
									{
										ImGui::InputFloat3("Position", &transformComp->Position.x, "%.3f", ImGuiInputTextFlags_CharsDecimal);
										ImGui::InputFloat3("Rotation", &transformComp->Rotation.x, "%.3f", ImGuiInputTextFlags_CharsDecimal);
										ImGui::InputFloat3("Scale", &transformComp->Scale.x, "%.3f", ImGuiInputTextFlags_CharsDecimal);

										ImGui::TreePop();
									}
								}
								if (CameraComponent* cameraComp = currentScene->GetComponent<CameraComponent>(entID))
								{
									TPS_CALL_ONCE(ImGui::SetNextItemOpen, true);
									if (ImGui::TreeNode(TempusUtils::GetClassDebugName<CameraComponent>()))
									{
										ImGui::Text("Project Type: %s", cameraComp->ProjectionType == CamProjectionType::Perspective ? "Perspective" : "Orthographic");
										ImGui::SliderFloat("FOV", &cameraComp->Fov, 1.0f, 179.0f, "%.3f");
										ImGui::Text("Ortho Size: %.1f", cameraComp->OrthoSize);
										ImGui::Text("Near Clip: %.1f", cameraComp->NearClip);
										ImGui::Text("Far Clip: %.1f", cameraComp->FarClip);

										ImGui::TreePop();
									}
								}
								if (StaticMeshComponent* meshComp = currentScene->GetComponent<StaticMeshComponent>(entID))
								{
									if (ImGui::TreeNode(TempusUtils::GetClassDebugName<StaticMeshComponent>()))
									{
										ImGui::Text("WIP");
										ImGui::TreePop();
									}
								}
								
								ImGui::TreePop();
							}
						}
						else
						{
							ImGui::Text(std::format("#{}: {}", entID, currentScene->GetEntityName(entID)).c_str());
						}
					}

					ImGui::TreePop();
				}
			}
		ImGui::End();
	}
	
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
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	
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
		m_SwapChainImageViews[i] = CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
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


	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create render pass!");
	}
}

void Tempus::Renderer::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// Can refer to an array of uniforms. We only have 1
	uboLayoutBinding.descriptorCount = 1;
	// Only accessing this uniform from the vertex shader
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to create descriptor set layout!");
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

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


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
	// 1 Descriptor set
	pipelineLayoutInfo.setLayoutCount = 1; 
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional
	
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
	pipelineInfo.pDepthStencilState = &depthStencil;
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
		std::array<VkImageView, 2> attachments =
		{
			m_SwapChainImageViews[i],
			m_DepthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
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

void Tempus::Renderer::CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();
	CreateImage(m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);

	m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Tempus::Renderer::CreateTextureImage()
{
	int texWidth, texHeight, texChannels;
	const char* path = "Tempus/res/textures/LogoTex.png";
	stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) 
	{
		TPS_CORE_CRITICAL("Failed to load texture image! {0}", path);
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Create buffer to store image data
	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, imageSize);
	vkUnmapMemory(m_Device, stagingBufferMemory);

	stbi_image_free(pixels);

	CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);

	// Image needs to be transitioned to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL before copying the buffer into it
	TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	// Copying data from staging buffer to texture image
	CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	// Transitioning to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL for shader access
	TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void Tempus::Renderer::CreateTextureImageView()
{
	m_TextureImageView = CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Tempus::Renderer::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);
	// Setting to max anisotropy value based on hardware
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	// Fallback color when sampling beyond clamped image
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	// Basically every real world application uses normalized UV's
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to create texture sampler!");
	}
}

void Tempus::Renderer::CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	// Staging buffer that can be accessed from host
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Moving buffer from host to device
	void* data;
	vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_Device, stagingBufferMemory);

	// Vertex buffer for direct drawing that is inaccessible from host
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

	// Copying data from staging buffer to vertex buffer
	CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void Tempus::Renderer::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_Device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

	CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void Tempus::Renderer::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	m_UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_UniformBuffers[i], m_UniformBuffersMemory[i]);

		vkMapMemory(m_Device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
	}
}

void Tempus::Renderer::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	// For vertex uniform buffer
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	// For texture image sampler
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to create descriptor pool!");
	}
}

void Tempus::Renderer::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

	if (vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_TextureImageView;
		imageInfo.sampler = m_TextureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr; // Optional
		descriptorWrites[0].pTexelBufferView = nullptr; // Optional

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_DescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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
	// Setting fence to be signaled on creation for first call of DrawFrame()
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

void Tempus::Renderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
}

void Tempus::Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

void Tempus::Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	EndSingleTimeCommands(commandBuffer);
}

void Tempus::Renderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to allocate image memory!");
	}

	vkBindImageMemory(m_Device, image, imageMemory, 0);
}

VkImageView Tempus::Renderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		TPS_CORE_CRITICAL("Failed to create image view!");
	}

	return imageView;
}

void Tempus::Renderer::RecreateSwapChain()
{
	TPS_CORE_INFO("Recreating swapchain!");

	vkDeviceWaitIdle(m_Device);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateDepthResources();
	CreateFrameBuffers();
}

void Tempus::Renderer::InitImGui()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
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

	// Order of clear values needs to match order of attachments
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{m_ClearColor[0],m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]}};
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_RenderPass;
	renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_SwapChainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	// Begin render pass
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	// Bind the pipeline
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

	VkBuffer vertexBuffers[] = { m_VertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[m_CurrentFrame], 0, nullptr);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
	{
		TPS_CORE_CRITICAL("Failed to record command buffer!");
	}
}

VkCommandBuffer Tempus::Renderer::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Tempus::Renderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_GraphicsQueue);

	vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void Tempus::Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	// Queue family ownership is not being transferred
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	
	// If transitioning from undefined to transfer
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		// Transfer writes don't need to wait on anything
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		// Earliest possible pipeline stage
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// If transitioning from transfer to shader read
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		// Shader should wait on transfer writes
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		TPS_CORE_CRITICAL("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(commandBuffer);
}

VkShaderModule Tempus::Renderer::CreateShaderModule(const std::vector<unsigned char>& code)
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

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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

VkFormat Tempus::Renderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
	VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	
	TPS_CORE_CRITICAL("Failed to find supported format!");
	return VkFormat();
}

VkFormat Tempus::Renderer::FindDepthFormat()
{
	return FindSupportedFormat(
		{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
}

bool Tempus::Renderer::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t Tempus::Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
		{
			return i;
		}
	}

	TPS_CORE_CRITICAL("Failed to find suitable memory type!");
	return 0;
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
		// Nvidia vendor ID
		if (deviceProperties.vendorID == 0X10DE) 
		{
			ss << '\t' << "Driver Version: " << ((deviceDetails.driverVersion >> 22) & 0x3FF) << '.' << ((deviceDetails.driverVersion >> 14) & 0xFF) << '.' << ((deviceDetails.driverVersion >> 6) & 0xFF) << '\n';
		}
		else 
		{
			ss << '\t' << "Driver Version: " << VK_VERSION_MAJOR(deviceDetails.driverVersion) << '.' << VK_VERSION_MINOR(deviceDetails.driverVersion) << '.' << VK_VERSION_PATCH(deviceDetails.driverVersion) <<'\n';
		}
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

    std::set<VkFormat> uniqueFormats;
    for (const auto& surfaceFormat : details.formats)
    {
        uniqueFormats.insert(surfaceFormat.format);
    }
    ss << "Formats: \n";
    for (const auto& format : uniqueFormats)
    {
        ss << "\tFormat: " << string_VkFormat(format) << '\n';
    }

    std::set<VkColorSpaceKHR> uniqueColorSpaces;
    for (const auto& surfaceFormat : details.formats)
    {
        uniqueColorSpaces.insert(surfaceFormat.colorSpace);
    }
    ss << "Available Color spaces: \n";
    for (const auto& colorSpace : uniqueColorSpaces)
    {
        ss << "\t" << string_VkColorSpaceKHR(colorSpace) << '\n';
    }

    std::set<VkPresentModeKHR> uniquePresentModes(details.presentModes.begin(), details.presentModes.end());
    ss << "Available Present modes: \n";
    for (const auto& presentMode : uniquePresentModes)
    {
        ss << "\t" << string_VkPresentModeKHR(presentMode) << '\n'; // Convert VkPresentModeKHR to string
    }

    TPS_CORE_INFO(ss.str());
}

void Tempus::Renderer::CleanupSwapChain()
{
	vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
	vkDestroyImage(m_Device, m_DepthImage, nullptr);
	vkFreeMemory(m_Device, m_DepthImageMemory, nullptr);
	
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

	vkDestroySampler(m_Device, m_TextureSampler, nullptr);
	vkDestroyImageView(m_Device, m_TextureImageView, nullptr);

	vkDestroyImage(m_Device, m_TextureImage, nullptr);
	vkFreeMemory(m_Device, m_TextureImageMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
		vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);

	vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
	vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
	vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);

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

	TPS_CORE_INFO("Renderer Cleaned");
}
