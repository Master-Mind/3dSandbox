#pragma once
#include "volk.h"
#include <vector>
#include <optional>
#include <glm/mat4x4.hpp>
#include <array>

struct GLFWwindow;

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

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
	static void RecreateSwapChain();
	static void CleanupSwapChain();

	static void CreateDescriptorSetLayout();
	static void CreateRenderPass();
	static void CreateGraphicsPipeline();
	static void CompileShaders();
	static VkShaderModule CreateShaderModule(std::vector<char>& code);

	static void CreateFramebuffers();
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	static void LoadModel();
	static void CreateVertexBuffer();
	static void CreateIndexBuffer();
	static void CreateUniformBuffers();
	static void CreateDescriptorPool();
	static void CreateDescriptorSets();
	static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkBuffer& buffer, 
		VkDeviceMemory& bufferMemory);
	static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	static void CreateCommandPool();
	static void CreateCommandBuffers();
	static void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	static VkCommandBuffer BeginSingleTimeCommands();
	static void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	static void CreateDepthResources();
	static VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	static VkFormat FindDepthFormat();
	static bool HasStencilComponent(VkFormat format);

	static void CreateTextureImage();
	static void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	static void TransitionImageLayout(VkImage image, VkFormat format, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout);
	static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static VkImageView CreateImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags);
	static void CreateTextureImageView();
	static void CreateTextureSampler();
	static void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	static void CreateSyncObjects();

	static VkSampleCountFlagBits GetMaxUsableSampleCount();
	static void CreateColorResources();

	static void DrawFrame();
	static void UpdateUniformBuffer(uint32_t currentImage);

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
	inline static VkDescriptorSetLayout _descriptorSetLayout;
	inline static VkDescriptorPool _descriptorPool;
	inline static std::vector<VkDescriptorSet> _descriptorSets;
	inline static VkPipelineLayout _pipelineLayout;
	inline static VkPipeline _graphicsPipeline;

	inline static std::vector<VkFramebuffer> _swapChainFramebuffers;

	//TODO: Use one vk buffer per model for vertices and indices
	inline static std::vector<Vertex> _vertices;
	inline static std::vector<uint32_t> _indices;
	inline static VkBuffer _vertexBuffer{};
	inline static VkDeviceMemory _vertexBufferMemory{};
	inline static VkBuffer _indexBuffer{};
	inline static VkDeviceMemory _indexBufferMemory{};

	inline static std::vector<VkBuffer> _uniformBuffers;
	inline static std::vector<VkDeviceMemory> _uniformBuffersMemory;
	inline static std::vector<void*> _uniformBuffersMapped;

	inline static VkCommandPool _commandPool;
	inline static std::vector<VkCommandBuffer> _commandBuffers;

	inline static VkBuffer _stagingBuffer;
	inline static VkDeviceMemory _stagingBufferMemory;
	inline static uint32_t _mipLevels;
	inline static VkImage _textureImage;
	inline static VkDeviceMemory _textureImageMemory;
	inline static VkImageView _textureImageView;
	inline static VkSampler _textureSampler;

	inline static VkImage _depthImage;
	inline static VkDeviceMemory _depthImageMemory;
	inline static VkImageView _depthImageView;

	inline static std::vector<VkSemaphore> _imageAvailableSemaphores;
	inline static std::vector<VkSemaphore> _renderFinishedSemaphores;
	inline static std::vector<VkFence> _inFlightFences;
	constexpr static int MAX_FRAMES_IN_FLIGHT = 2;
	inline static uint32_t currentFrame = 0;

	inline static bool _framebufferResized = false;

	inline static VkSampleCountFlagBits _msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	inline static VkImage _colorImage;
	inline static VkDeviceMemory _colorImageMemory;
	inline static VkImageView _colorImageView;

	static glm::mat4 _model;
	static glm::mat4 _view;
	static glm::mat4 _proj;
};
