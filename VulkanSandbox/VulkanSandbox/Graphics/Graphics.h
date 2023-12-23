#pragma once
#include <vector>
#include <optional>
#include <glm/mat4x4.hpp>
#include <array>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static vk::VertexInputBindingDescription getBindingDescription()
	{
		vk::VertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		
		return bindingDescription;
	}

	static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
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

	static bool CheckDeviceExtensionSupport(vk::PhysicalDevice device);
	static void PickPhysicalDevice();
	static bool IsPhysicalDeviceSuitable(vk::PhysicalDevice device);

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool ValidForRendering() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	static QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device);

	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities{};
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	static SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device);
	static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
	static vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

	static void CreateLogicalDevice();

	static void CreateVMAAllocator();

	static void CreateSwapChain();
	static void CreateImageViews();
	static void RecreateSwapChain();
	static void CleanupSwapChain();

	static void CreateDescriptorSetLayout();
	static void CreateRenderPass();
	static void CreateGraphicsPipeline();
	static void CompileShaders();
	static vk::ShaderModule CreateShaderModule(std::vector<char>& code);

	static void CreateFramebuffers();
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	static void LoadModel();
	static void CreateVertexBuffer();
	static void CreateIndexBuffer();
	static void CreateUniformBuffers();
	static void CreateDescriptorPool();
	static void CreateDescriptorSets();
	static void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, 
		vk::MemoryPropertyFlags properties, vk::Buffer& buffer, 
		VmaAllocation& bufferMemory, uint32_t memoryTypeBits = 0);
	static void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
	static uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	static void CreateCommandPool();
	static void CreateCommandBuffers();
	static void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
	static vk::CommandBuffer BeginSingleTimeCommands();
	static void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);

	static void CreateDepthResources();
	static vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
	static vk::Format FindDepthFormat();
	static bool HasStencilComponent(vk::Format format);

	static void CreateTextureImage();
	static void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits samples,
		vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties, vk::Image& image, VmaAllocation& imageMemory);
	static void TransitionImageLayout(vk::Image image, vk::Format format, uint32_t mipLevels, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	static void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
	static vk::ImageView CreateImageView(vk::Image image, vk::Format format, uint32_t mipLevels, vk::ImageAspectFlags aspectFlags);
	static void CreateTextureImageView();
	static void CreateTextureSampler();
	static void GenerateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	static void CreateSyncObjects();

	static vk::SampleCountFlagBits GetMaxUsableSampleCount();
	static void CreateColorResources();

	static void DrawFrame();
	static void UpdateUniformBuffer(uint32_t currentImage);

	//glfw
	inline static GLFWwindow* _window = nullptr;
	inline static int _width = 800;
	inline static int _height = 600;

	//vulkan
	inline static vk::Instance _instance;
	const static std::vector<const char*> _validationLayers;
	inline static vk::DebugUtilsMessengerEXT _debugMessenger{};

	inline static vk::PhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	inline static vk::Device  _device = VK_NULL_HANDLE;
	inline static QueueFamilyIndices _queueFamilyIndices;
	inline static vk::Queue _graphicsQueue{};
	inline static vk::Queue _presentQueue{};
	const static std::vector<const char*> _deviceExtensions;

	inline static vk::SwapchainKHR _swapChain{};
	inline static std::vector<vk::Image> _swapChainImages;
	inline static vk::Format _swapChainImageFormat{};
	inline static vk::Extent2D _swapChainExtent{};
	inline static std::vector<vk::ImageView> _swapChainImageViews;

	inline static vk::SurfaceKHR _surface{};

	inline static vk::RenderPass _renderPass;
	inline static vk::DescriptorSetLayout _descriptorSetLayout;
	inline static vk::DescriptorPool _descriptorPool;
	inline static std::vector<vk::DescriptorSet> _descriptorSets;
	inline static vk::PipelineLayout _pipelineLayout;
	inline static vk::Pipeline _graphicsPipeline;

	inline static std::vector<vk::Framebuffer> _swapChainFramebuffers;

	//TODO: Use one vk buffer per model for vertices and indices
	inline static std::vector<Vertex> _vertices;
	inline static std::vector<uint32_t> _indices;
	inline static vk::Buffer _vertexBuffer{};
	inline static VmaAllocation _vertexBufferMemory{};
	inline static vk::Buffer _indexBuffer{};
	inline static VmaAllocation _indexBufferMemory{};

	inline static std::vector<vk::Buffer> _uniformBuffers;
	inline static std::vector<VmaAllocation> _uniformBuffersMemory;
	inline static std::vector<void*> _uniformBuffersMapped;

	inline static vk::CommandPool _commandPool;
	inline static std::vector<vk::CommandBuffer> _commandBuffers;

	inline static vk::Buffer _stagingBuffer;
	inline static VmaAllocation _stagingBufferMemory;
	inline static uint32_t _mipLevels;
	inline static vk::Image _textureImage;
	inline static VmaAllocation _textureImageMemory;
	inline static vk::ImageView _textureImageView;
	inline static vk::Sampler _textureSampler;

	inline static vk::Image _depthImage;
	inline static VmaAllocation _depthImageMemory;
	inline static vk::ImageView _depthImageView;

	inline static std::vector<vk::Semaphore> _imageAvailableSemaphores;
	inline static std::vector<vk::Semaphore> _renderFinishedSemaphores;
	inline static std::vector<vk::Fence> _inFlightFences;
	constexpr static int MAX_FRAMES_IN_FLIGHT = 2;
	inline static uint32_t currentFrame = 0;

	inline static bool _framebufferResized = false;

	inline static vk::SampleCountFlagBits _msaaSamples = vk::SampleCountFlagBits::e1;
	inline static vk::Image _colorImage;
	inline static VmaAllocation _colorImageMemory;
	inline static vk::ImageView _colorImageView;

	static glm::mat4 _model;
	static glm::mat4 _view;
	static glm::mat4 _proj;

	inline static VmaAllocator _allocator;
};
