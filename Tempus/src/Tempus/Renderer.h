// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"

#include "sdl/SDL.h"
#include <vector>
#include "vulkan/vulkan.h"
#include <optional>
#include "Log.h"
#include "Events/IEventListener.h"
#include "glm/glm.hpp"
#include <array>

#ifdef TPS_PLATFORM_MAC
#include "vulkan/vulkan_macos.h"
#include "vulkan/vulkan_metal.h"
#endif


namespace Tempus {

	struct TEMPUS_API Vertex {

		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription GetBindingDescription() 
		{
			VkVertexInputBindingDescription bindingDescription{};

			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() 
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			// Vertex position attribute
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			// Vertex color attribute
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
	};

	class TEMPUS_API Renderer : public IEventListener {

	public:

		Renderer();
		~Renderer();

		void Update();

		bool Init(class Window* window);

		void SetClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		const int MAX_FRAMES_IN_FLIGHT = 2;

	private:

		const std::vector<Vertex> vertices = 
		{
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = 
		{
			0, 1, 2, 2, 3, 0
		};

		virtual void OnEvent(const SDL_Event& event) override;

		float m_ClearColor[4] = {0.25f, 0.5f, 0.1f, 0.0f};

		// Struct for potential queue families
		// Currently only searching for graphics queue family but will add more later
		struct QueueFamilyIndices
		{
			// Optional value to represent if queue family exists
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool IsComplete() 
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities = { 0 };
			// Colour format and bits per pixel
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		void DrawFrame();
		void DrawImGui();

		void CreateVulkanInstance();
		void SetupDebugMessenger();
		void CreateSurface(class Window* window);
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateCommandBuffer();
		void CreateSyncObjects();

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		void RecreateSwapChain();

		void InitImGui();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		bool IsDeviceSuitable(VkPhysicalDevice device);
		uint32_t GetDeviceScore(VkPhysicalDevice device);
		bool CheckValidationLayerSupport();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		// Swap chain support checks
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		// Debug logging functions
		void LogExtensionsAndLayers();
		void LogDeviceInfo();
		void LogSwapchainDetails(const SwapChainSupportDetails& details);

		void CleanupSwapChain();
		void Cleanup();

	private:

		Window* m_Window = nullptr;

		VkInstance m_VkInstance = VK_NULL_HANDLE;
		VkSurfaceKHR m_VkSurface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkImageView> m_SwapChainImageViews;
		VkFormat m_SwapChainImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_SwapChainExtent = { 0 };

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		uint32_t m_CurrentFrame = 0;

		VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

		bool m_FramebufferResized = false;

		VkDescriptorPool m_ImguiPool = VK_NULL_HANDLE;

		// Standard validation layer
		const std::vector<const char*> m_ValidationLayers = 
		{
			DESIRED_VK_LAYER
		};

		// Required device extensions
		const std::vector<const char*> m_DeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

#ifdef TPS_DEBUG
		const bool m_bEnableValidationLayers = true;
#else
		const bool m_bEnableValidationLayers = false;
#endif

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		struct DeviceDetails
		{
			std::string name;
			uint32_t id;
			std::string type;
			uint32_t driverVersion;
			uint32_t apiVersion;
			uint32_t vendorId;
		};

		DeviceDetails m_DeviceDetails;

	private:

		// Callback function for validation layer debug messages
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
		{

			switch (messageSeverity) 
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				//TPS_CORE_INFO("Validation Layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				TPS_CORE_INFO("Validation Layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				TPS_CORE_WARN("Validation Layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				TPS_CORE_ERROR("Validation Layer: {0}", pCallbackData->pMessage);
				break;
			default:
				break;
			}

			return VK_FALSE;
		}

		// These functions are apart of extensions and therefore must be manually loaded
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		 const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) 
			{
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			} 
			else 
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

			if (func != nullptr) 
			{
				func(instance, debugMessenger, pAllocator);
			}
		}

	};

}