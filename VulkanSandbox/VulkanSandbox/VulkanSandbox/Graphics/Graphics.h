#pragma once
#include "volk.h"
#include <vector>
#include <optional>

struct GLFWwindow;

class Graphics
{
public:
	static void Init();
	static bool ShouldClose();
	static void Update();
	static void DeInit();
private:
	static void CreateInstance();
	static bool CheckValidationLayerSupport();
	static std::vector<const char*> GetRequiredExtensions();

	static void SetupDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	static void SetupDebugMessenger();

	static void CreateSurface();

	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	static void PickPhysicalDevice();
	static bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool ValidForRendering() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	static void CreateLogicalDevice();

	static void CreateSwapChain();
	static void CreateImageViews();

	static void CreateRenderPass();
	static void CreateGraphicsPipeline();
	static void CompileShaders();
	static VkShaderModule CreateShaderModule(std::vector<char>& code);

	static void CreateFramebuffers();

	static void CreateCommandPool();
	static void CreateCommandBuffers();
	static void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	static void CreateSyncObjects();

	static void DrawFrame();

	//glfw
	inline static GLFWwindow* _window = nullptr;
	inline static int _width = 800;
	inline static int _height = 600;

	//vulkan
	inline static VkInstance _instance;
	const static std::vector<const char*> _validationLayers;
	inline static VkDebugUtilsMessengerEXT _debugMessenger{};

	inline static VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	inline static VkDevice  _device = VK_NULL_HANDLE;
	inline static QueueFamilyIndices _queueFamilyIndices;
	inline static VkQueue _graphicsQueue{};
	inline static VkQueue _presentQueue{};
	const static std::vector<const char*> _deviceExtensions;

	inline static VkSwapchainKHR _swapChain{};
	inline static std::vector<VkImage> _swapChainImages;
	inline static VkFormat _swapChainImageFormat{};
	inline static VkExtent2D _swapChainExtent{};
	inline static std::vector<VkImageView> _swapChainImageViews;

	inline static VkSurfaceKHR _surface{};

	inline static VkRenderPass _renderPass;
	inline static VkPipelineLayout _pipelineLayout;
	inline static VkPipeline _graphicsPipeline;

	inline static std::vector<VkFramebuffer> _swapChainFramebuffers;

	inline static VkCommandPool _commandPool;
	inline static std::vector<VkCommandBuffer> _commandBuffers;

	inline static std::vector<VkSemaphore> _imageAvailableSemaphores;
	inline static std::vector<VkSemaphore> _renderFinishedSemaphores;
	inline static std::vector<VkFence> _inFlightFences;
	constexpr static int MAX_FRAMES_IN_FLIGHT = 2;
	inline static uint32_t currentFrame = 0;
};
