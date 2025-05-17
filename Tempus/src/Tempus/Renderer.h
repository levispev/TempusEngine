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
#include <bitset>

#ifdef TPS_PLATFORM_MAC
#include "vulkan/vulkan_macos.h"
#include "vulkan/vulkan_metal.h"
#endif


namespace Tempus {

	struct TEMPUS_API Vertex {

		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription GetBindingDescription() 
		{
			VkVertexInputBindingDescription bindingDescription{};

			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() 
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			// Vertex position attribute
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			// Vertex color attribute
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class TEMPUS_API Renderer : public IEventListener {

	public:

		Renderer();
		virtual ~Renderer();

		void Update();

		bool Init(class Window* window);

		void SetClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		const int MAX_FRAMES_IN_FLIGHT = 2;

	private:

		const std::vector<Vertex> vertices = {
			// 1st cube
			// Front face 
			{{ 0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 0 bottom-left
			{{ 0.5f, 0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 1 top-left
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 2 bottom-right
			{{-0.5f, 0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 3 top-right
			
			// Back face 
			{{ 0.5f,-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 4 bottom-left
			{{ 0.5f,-0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 5 top-left
			{{-0.5f,-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 6 bottom-right
			{{-0.5f,-0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 7 top-right
			
			// Left face 
			{{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 8 bottom-left
			{{0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 9 top-left
			{{0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 10 bottom-right
			{{0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 11 top-right
			
			// Right face 
			{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 12 bottom-left
			{{-0.5f, 0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 13 top-left
			{{-0.5f,-0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 14 bottom-right
			{{-0.5f,-0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 15 top-right
			
			// Top face 
			{{ 0.5f, 0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 16 bottom-left
			{{ 0.5f,-0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 17 top-left
			{{-0.5f, 0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 18 bottom-right
			{{-0.5f,-0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 19 top-right
			
			// Bottom face 
			{{ 0.5f,-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 20 bottom-left
			{{ 0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 21 top-left
		    {{-0.5f,-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 22 bottom-right
		    {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 23 top-right

			// 2nd cube
			// Front face 
			{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 24 bottom-left
			{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 25 top-left
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 26 bottom-right
			{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 27 top-right

			// Back face 
			{{ 0.5f, -1.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 28 bottom-left
			{{ 0.5f, -1.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 29 top-left
			{{-0.5f, -1.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 30 bottom-right
			{{-0.5f, -1.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 31 top-right

			// Left face 
			{{0.5f, -1.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 32 bottom-left
			{{0.5f, -1.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 33 top-left
			{{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 34 bottom-right
			{{0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 35 top-right

			// Right face 
			{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 36 bottom-left
			{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 37 top-left
			{{-0.5f, -1.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 38 bottom-right
			{{-0.5f, -1.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 39 top-right

			// Top face 
			{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 40 bottom-left
			{{ 0.5f, -1.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 41 top-left
			{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 42 bottom-right
			{{-0.5f, -1.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 43 top-right

			// Bottom face 
			{{ 0.5f, -1.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 44 bottom-left
			{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 45 top-left
			{{-0.5f, -1.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 46 bottom-right
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 47 top-right


		};

		const std::vector<uint16_t> indices = {
		    // 1st Cube
		    // Front face
		    0, 1, 2,    2, 1, 3,
		    // Back face
		    6, 7, 4,    4, 7, 5,
		    // Left face
		    8, 9, 10,   10, 9, 11,
		    // Right face
		    12, 13, 14, 14, 13, 15,
		    // Top face
		    16, 17, 18, 18, 17, 19,
		    // Bottom face
		    20, 21, 22, 22, 21, 23,

			// 2nd Cube
			// Front face
			24, 25, 26,    26, 25, 27,
			// Back face
			30, 31, 28,    28, 31, 29,
			// Left face
			32, 33, 34,    34, 33, 35,
			// Right face
			36, 37, 38,    38, 37, 39,
			// Top face
			40, 41, 42,    42, 41, 43,
			// Bottom face
			44, 45, 46,    46, 45, 47,
		};

		virtual void OnEvent(const SDL_Event& event) override;

		float m_ClearColor[4] = {0.25f, 0.5f, 0.1f, 0.0f};

		// Temporary input testing
		std::bitset<8> m_InputBits;
		glm::vec3 m_CamPos = glm::vec3(0.0f, -3.0f, 0.0f);

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
		void UpdateUniformBuffer(uint32_t currentImage);
		void DrawImGui();

		void CreateVulkanInstance();
		void SetupDebugMessenger();
		void CreateSurface(class Window* window);
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateDepthResources();
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffer();
		void CreateSyncObjects();

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		void RecreateSwapChain();

		void InitImGui();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		// Transitions an images layout type using a buffer memory barrier
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		VkShaderModule CreateShaderModule(const std::vector<unsigned char>& code);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		bool IsDeviceSuitable(VkPhysicalDevice device);
		uint32_t GetDeviceScore(VkPhysicalDevice device);
		bool CheckValidationLayerSupport();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);

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
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
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

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_UniformBuffersMapped;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		// Texture image
		VkImage m_TextureImage = VK_NULL_HANDLE;
		VkDeviceMemory m_TextureImageMemory = VK_NULL_HANDLE;
		VkImageView m_TextureImageView = VK_NULL_HANDLE;
		VkSampler m_TextureSampler = VK_NULL_HANDLE;
		// Depth image
		VkImage m_DepthImage = VK_NULL_HANDLE;
		VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
		VkImageView m_DepthImageView = VK_NULL_HANDLE;

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