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
	static void PickPhysicalDevice();
	static bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
	};
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	static void CreateLogicalDevice();

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
	inline static VkQueue _graphicsQueue;
};
