#include "Graphics.h"

#include <GLFW/glfw3.h>

#include "../Utils/CLogger.h"

using namespace std;

const vector<const char*> Graphics::_validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    const char* vkMsgTypeNames[] = { "General",
    "Validation",
    "Performance"};
    
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: 
        Log("Vulkan Verbose", { {"Type", vkMsgTypeNames[messageType]} ,
            {"Message", pCallbackData->pMessage} });
        break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        Log("Vulkan Info", { {"Type", vkMsgTypeNames[messageType]} ,
            {"Message", pCallbackData->pMessage} });
        break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: 
        Error("Vulkan Warning", { {"Type", vkMsgTypeNames[messageType]} ,
            {"Message", pCallbackData->pMessage} });
        break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: 
        Assert(false, "Vulkan Error!", { {"Type", vkMsgTypeNames[messageType]} ,
            {"Message", pCallbackData->pMessage} });
        break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
	default:
		Assert(false, "Unkown Vulkan Message severity!", {{"Severity", messageSeverity}, 
            {"Type", vkMsgTypeNames[messageType]} ,
			{"Message", pCallbackData->pMessage} });
	}

    return VK_FALSE;
}

void Graphics::Init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(_width, _height, "Vulkan window", nullptr, nullptr);
    
    VkResult result = volkInitialize();

    Assert(result == VK_SUCCESS, "Volk failed to initialize!", {{"Error code", result}});

    CreateInstance();
    SetupDebugMessenger();
    PickPhysicalDevice();
    CreateLogicalDevice();
}

void Graphics::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;

    vector<const char*> reqExtensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size());
    createInfo.ppEnabledExtensionNames = reqExtensions.data(); uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    vector<VkExtensionProperties> extensions(extensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    Assert(!enableValidationLayers || CheckValidationLayerSupport(), "Validation layers requested, but not available!");

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if constexpr (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
        createInfo.ppEnabledLayerNames = _validationLayers.data();

        SetupDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
    volkLoadInstance(_instance);

    Assert(result == VK_SUCCESS, "Failed to create Vulkan Instance", {{"Error code", result}});
}

bool Graphics::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : _validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> Graphics::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if constexpr (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Graphics::SetupDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional
}

void Graphics::SetupDebugMessenger()
{
    if constexpr (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};

    SetupDebugMessengerCreateInfo(createInfo);

    VkResult result = vkCreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger);

    Assert(result == VK_SUCCESS, "Failed to create debug messenger!", {{"Error code", result}});
}

bool Graphics::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        deviceFeatures.geometryShader &&
        FindQueueFamilies(device).graphicsFamily.has_value();
}

void Graphics::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    Assert(deviceCount, "Failed to find GPUs with Vulkan support!");

    vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices) 
    {
        if (IsPhysicalDeviceSuitable(device))
        {
            _physicalDevice = device;
            break;
        }
    }

    Assert(_physicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU!");

    _queueFamilyIndices = FindQueueFamilies(_physicalDevice);
}

Graphics::QueueFamilyIndices Graphics::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) 
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        i++;
    }

    return indices;
}

void Graphics::CreateLogicalDevice()
{
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = _queueFamilyIndices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};
    vkGetPhysicalDeviceFeatures(_physicalDevice, &deviceFeatures);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
        createInfo.ppEnabledLayerNames = _validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);

    Assert(result == VK_SUCCESS, "Failed to create logical device!");

    vkGetDeviceQueue(_device, _queueFamilyIndices.graphicsFamily.value(), 0, &_graphicsQueue);
}

bool Graphics::ShouldClose()
{
    return glfwWindowShouldClose(_window);
}

void Graphics::Update()
{
    glfwPollEvents();
}

void Graphics::DeInit()
{
    if constexpr (enableValidationLayers)
    {
        vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    }

    vkDestroyDevice(_device, nullptr);
    vkDestroyInstance(_instance, nullptr);
    glfwDestroyWindow(_window);

    glfwTerminate();
}