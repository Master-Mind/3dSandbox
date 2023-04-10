#include "Graphics.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include <algorithm>
#include <set>
#include <GLFW/glfw3.h>
#include <filesystem>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <shaderc/shaderc.hpp>

#include "../Utils/CLogger.h"
#include "../Utils/utils.h"

using namespace std;
using namespace glm;

mat4 Graphics::_model = mat4(1);
mat4 Graphics::_view = mat4(1);
mat4 Graphics::_proj = mat4(1);

const vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

const vector<const char*> Graphics::_validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const vector<const char*> Graphics::_deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
		"",
        "",
	"Performance",
        "" ,
        "" ,
        "",
    "Device Address Binding"};

    VkDebugUtilsMessageSeverityFlagBitsEXT minSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    if (messageSeverity < minSeverity)
    {
        return VK_FALSE;
    }

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
    CompileShaders();
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    _window = glfwCreateWindow(_width, _height, "Vulkan window", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
    
    //VkResult result = volkInitialize();

    //Assert(result == VK_SUCCESS, "Volk failed to initialize!", {{"Error code", result}});

    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    LoadModel();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void Graphics::CreateInstance()
{
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;

    vector<const char*> reqExtensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size());
    createInfo.ppEnabledExtensionNames = reqExtensions.data(); uint32_t extensionCount = 0;

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

    vk::Result result = vk::createInstance(&createInfo, nullptr, &_instance);
    //volkLoadInstance(_instance);

    Assert(result == vk::Result::eSuccess, "Failed to create Vulkan Instance", {{"Error code", static_cast<uint32_t>(result)}});
    
    VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance);
}

bool Graphics::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vk::Result result = vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

    Assert(result == vk::Result::eSuccess, "Failed to enumerate instance layer properties!", { {"Error code", static_cast<uint32_t>(result)} });

    vector<vk::LayerProperties> availableLayers(layerCount);
    result = vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    Assert(result == vk::Result::eSuccess, "Failed to enumerate instance layer properties!", { {"Error code", static_cast<uint32_t>(result)} });

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

    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};

    SetupDebugMessengerCreateInfo(createInfo);

    vk::Result result = _instance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &_debugMessenger);

    Assert(result == vk::Result::eSuccess, "Failed to create debug messenger!", {{"Error code", static_cast<uint32_t>(result)}});
}

bool Graphics::CheckDeviceExtensionSupport(vk::PhysicalDevice device)
{
    uint32_t extensionCount;
    vk::Result result = device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);

    Assert(result == vk::Result::eSuccess, "Failed to enumerate device extension properties!", { {"Error code", static_cast<uint32_t>(result)} });

    vector<vk::ExtensionProperties> availableExtensions(extensionCount);
    result = device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    Assert(result == vk::Result::eSuccess, "Failed to enumerate device extension properties!", { {"Error code", static_cast<uint32_t>(result)} });

    set<string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool Graphics::IsPhysicalDeviceSuitable(vk::PhysicalDevice device)
{
    vk::PhysicalDeviceProperties deviceProperties;
    device.getProperties(&deviceProperties);

    vk::PhysicalDeviceFeatures deviceFeatures;
    device.getFeatures(&deviceFeatures);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    bool swapChainAdequate = false;

    if (extensionsSupported)
    {
        SwapChainSupportDetails details = QuerySwapChainSupport(device);
        swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
    }

	return deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
        deviceFeatures.geometryShader &&
        FindQueueFamilies(device).ValidForRendering() &&
        extensionsSupported &&
        swapChainAdequate &&
        deviceFeatures.samplerAnisotropy;
}

void Graphics::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vk::Result result = _instance.enumeratePhysicalDevices(&deviceCount, nullptr);

    Assert(result == vk::Result::eSuccess && deviceCount, "Failed to find GPUs with Vulkan support!");

    vector<vk::PhysicalDevice> devices(deviceCount);
    result = _instance.enumeratePhysicalDevices(&deviceCount, devices.data());

    Assert(result == vk::Result::eSuccess, "Failed to find GPUs with Vulkan support!");

    for (vk::PhysicalDevice device : devices) 
    {
        if (IsPhysicalDeviceSuitable(device))
        {
            _physicalDevice = device;
            _msaaSamples = GetMaxUsableSampleCount();
            break;
        }
    }

    Assert(_physicalDevice, "Failed to find a suitable GPU!");

    _queueFamilyIndices = FindQueueFamilies(_physicalDevice);
}

Graphics::QueueFamilyIndices Graphics::FindQueueFamilies(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    device.getQueueFamilyProperties(&queueFamilyCount, nullptr);

    vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
    device.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) 
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
            break;
        }

        i++;
    }

	VkBool32 presentSupport = false;
    vk::Result result = device.getSurfaceSupportKHR(i, _surface, &presentSupport);

    Assert(result == vk::Result::eSuccess, "Failed to get surface support!");

    if (presentSupport)
    {
        indices.presentFamily = i;
    }

    return indices;
}

Graphics::SwapChainSupportDetails Graphics::QuerySwapChainSupport(vk::PhysicalDevice device)
{
    SwapChainSupportDetails details;

    vk::Result result = device.getSurfaceCapabilitiesKHR(_surface, &details.capabilities);

    Assert(result == vk::Result::eSuccess, "Failed to query swap chain support!");

    uint32_t formatCount;
    result = device.getSurfaceFormatsKHR(_surface, &formatCount, nullptr);

    Assert(result == vk::Result::eSuccess, "Failed to get surface format!");

    if (formatCount != 0) 
    {
        details.formats.resize(formatCount);
        result = device.getSurfaceFormatsKHR(_surface, &formatCount, details.formats.data());

        Assert(result == vk::Result::eSuccess, "Failed to get surface format!");
    }

    uint32_t presentModeCount;
    result = device.getSurfacePresentModesKHR(_surface, &presentModeCount, nullptr);

    Assert(result == vk::Result::eSuccess, "Failed to get surface present modes!");

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        result = device.getSurfacePresentModesKHR(_surface, &presentModeCount, details.presentModes.data());

        Assert(result == vk::Result::eSuccess, "Failed to get surface present modes!");
    }

    return details;
}

vk::SurfaceFormatKHR Graphics::ChooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && 
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR Graphics::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Graphics::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        vk::Extent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Graphics::CreateLogicalDevice()
{
    vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    set uniqueQueueFamilies = { _queueFamilyIndices.graphicsFamily.value(), _queueFamilyIndices.presentFamily.value() };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    _physicalDevice.getFeatures(&deviceFeatures);

    deviceFeatures.sampleRateShading = VK_TRUE;

    vk::DeviceCreateInfo createInfo{};

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = _deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
        createInfo.ppEnabledLayerNames = _validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    
    vk::Result result = _physicalDevice.createDevice(&createInfo, nullptr, &_device);

    Assert(result == vk::Result::eSuccess, "Failed to create logical device!");
    _device.getQueue(_queueFamilyIndices.graphicsFamily.value(), 0, &_graphicsQueue);
    _device.getQueue(_queueFamilyIndices.presentFamily.value(), 0, &_presentQueue);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(_device);
}

void Graphics::CreateSurface()
{
    VkSurfaceKHR rawSurface;
    VkResult result = glfwCreateWindowSurface(_instance, _window, nullptr, &rawSurface);

    _surface = rawSurface;

    Assert(result == VK_SUCCESS, "Failed to create surface!", { {"Error code", static_cast<uint32_t>(result)} });
}

void Graphics::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) 
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    uint32_t queueFamilyIndices[] = { _queueFamilyIndices.graphicsFamily.value(), _queueFamilyIndices.presentFamily.value() };

    if (_queueFamilyIndices.graphicsFamily != _queueFamilyIndices.presentFamily)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    vk::Result result = _device.createSwapchainKHR(&createInfo, nullptr, &_swapChain);

    Assert(result == vk::Result::eSuccess, "Failed to create swapchain!", {{"Error Code", static_cast<uint32_t>(result)}});

    result = _device.getSwapchainImagesKHR(_swapChain, &imageCount, nullptr);

    Assert(result == vk::Result::eSuccess, "Failed to get swapchain images!", { {"Error Code", static_cast<uint32_t>(result)} });

    _swapChainImages.resize(imageCount);

    result = _device.getSwapchainImagesKHR(_swapChain, &imageCount, _swapChainImages.data());

    Assert(result == vk::Result::eSuccess, "Failed to get swapchain images!", { {"Error Code", static_cast<uint32_t>(result)} });

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

void Graphics::CreateImageViews()
{
    _swapChainImageViews.resize(_swapChainImages.size());

	for (size_t i = 0; i < _swapChainImageViews.size(); i++)
    {
        _swapChainImageViews[i] = CreateImageView(_swapChainImages[i], _swapChainImageFormat, 
            static_cast<uint32_t>(vk::ImageAspectFlagBits::eColor), vk::ImageAspectFlagBits::eColor);
    }
}

void Graphics::RecreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);

    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }

    _device.waitIdle();

    CleanupSwapChain();

    CreateSwapChain();
    CreateColorResources();
    CreateDepthResources();
    CreateImageViews();
    CreateFramebuffers();
}

void Graphics::CleanupSwapChain()
{
    _device.destroyImageView(_colorImageView, nullptr);
    _device.destroyImage(_colorImage, nullptr);
    _device.freeMemory(_colorImageMemory, nullptr);
    _device.destroyImageView(_depthImageView, nullptr);
    _device.destroyImage(_depthImage, nullptr);
    _device.freeMemory(_depthImageMemory, nullptr);

    for (auto framebuffer : _swapChainFramebuffers)
    {
        _device.destroyFramebuffer(framebuffer, nullptr);
    }

    for (auto imageView : _swapChainImageViews)
    {
        _device.destroyImageView(imageView, nullptr);
    }

    _device.destroySwapchainKHR(_swapChain, nullptr);
}

void Graphics::CreateDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding mvpLayoutBinding{};
    mvpLayoutBinding.binding = 0;
    mvpLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    mvpLayoutBinding.descriptorCount = 1;
    mvpLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    mvpLayoutBinding.pImmutableSamplers = nullptr; // Optional

    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    array bindings = { mvpLayoutBinding, samplerLayoutBinding };

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    vk::Result result = _device.createDescriptorSetLayout(&layoutInfo, nullptr, &_descriptorSetLayout);
    Assert(result == vk::Result::eSuccess, "Failed to create render pass!", { {"Error Code", static_cast<uint32_t>(result)} });
}

void Graphics::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = _swapChainImageFormat;
    colorAttachment.samples = _msaaSamples;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = _swapChainImageFormat;
    colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = _msaaSamples;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    vk::SubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    std::array attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    vk::RenderPassCreateInfo renderPassInfo{};

    renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vk::Result result = _device.createRenderPass(&renderPassInfo, nullptr, &_renderPass);
    Assert(result == vk::Result::eSuccess, "Failed to create render pass!", {{"Error Code", static_cast<uint32_t>(result)}});
}

void Graphics::CreateGraphicsPipeline()
{
    vector<char> vertCode;
    vector<char> fragCode;
    loadWholeBinFile("./Build/Data/Shaders/VertShader.vert.spv", vertCode);
    loadWholeBinFile("./Build/Data/Shaders/FragShader.frag.spv", fragCode);

    vk::ShaderModule vertShaderModule = CreateShaderModule(vertCode);
    vk::ShaderModule fragShaderModule = CreateShaderModule(fragCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    vector dynamicStates = {
	    vk::DynamicState::eViewport,
	    vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_swapChainExtent.width);
    viewport.height = static_cast<float>(_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = _swapChainExtent;

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = _msaaSamples;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional
    multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
    multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrc1Alpha;
    colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = VK_TRUE;
    colorBlending.logicOp = vk::LogicOp::eAnd; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional


    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    vk::Result layoutResult = _device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &_pipelineLayout);

    Assert(layoutResult == vk::Result::eSuccess, "Failed to create pipeline layout!", {{"Error Code", static_cast<uint32_t>(layoutResult)}});

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    //depthStencil.front = {}; // Optional
    //depthStencil.back = {}; // Optional

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
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
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
	pipelineInfo.pDepthStencilState = &depthStencil;

    vk::Result pipelineResult = _device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline);
    Assert(pipelineResult == vk::Result::eSuccess, "Failed to create pipeline!", {{"Error Code", static_cast<uint32_t>(pipelineResult)}});

    _device.destroyShaderModule(fragShaderModule, nullptr);
    _device.destroyShaderModule(vertShaderModule, nullptr);
}

void Graphics::CompileShaders()
{
    const filesystem::path glslcPath = "C:\\VulkanSDK\\1.3.239.0\\Bin\\glslc.exe";
    shaderc::Compiler compiler;

    for (filesystem::directory_entry shaderIt : filesystem::directory_iterator("./Data/Shaders"))
    {
        if (shaderIt.is_regular_file() && (shaderIt.path().extension() == ".vert" || shaderIt.path().extension() == ".frag"))
        {
            filesystem::path outpath = "./Build/";
            outpath += shaderIt.path().parent_path().string().substr(2);
            outpath += '/';
            outpath += shaderIt.path().stem();
            outpath += shaderIt.path().extension();
            outpath += ".spv";

            if (!exists(outpath))
            {
                makeEmptyFile(outpath.string().c_str());
            }
            else if (last_write_time(outpath) > last_write_time(shaderIt.path()))
            {
                continue;
            }

            Log("Compiling shader...", {{"Shader", shaderIt.path().string()}, {"Output Dest.", outpath.string()}});

            vector<char> inData;
            loadWholeBinFile(shaderIt.path().string().c_str(), inData);

            shaderc_shader_kind shaderType = shaderc_glsl_infer_from_source;

            if (shaderIt.path().extension() == ".vert")
            {
                shaderType = shaderc_glsl_vertex_shader;
            }
            else if (shaderIt.path().extension() == ".frag")
            {
                shaderType = shaderc_glsl_fragment_shader;
            }
            else if (shaderIt.path().extension() == ".tesc")
            {
                shaderType = shaderc_glsl_tess_control_shader;
            }
            else if (shaderIt.path().extension() == ".tese")
            {
                shaderType = shaderc_glsl_tess_evaluation_shader;
            }
            else if (shaderIt.path().extension() == ".geom")
            {
                shaderType = shaderc_glsl_geometry_shader;
            }
            else if (shaderIt.path().extension() == ".comp")
            {
                shaderType = shaderc_glsl_compute_shader;
            }

            auto ret = compiler.CompileGlslToSpv(inData.data(), inData.size(), shaderType, shaderIt.path().string().c_str());
           
            if (ret.GetCompilationStatus() != shaderc_compilation_status_success)
            {
                Error("Shader compiler returned an error",{{"Return code", ret.GetCompilationStatus()}});

            	Error(ret.GetErrorMessage());
            }
        }
    }
}

vk::ShaderModule Graphics::CreateShaderModule(vector<char> &code)
{
	vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	vk::ShaderModule shaderModule;
    vk::Result result = _device.createShaderModule(&createInfo, nullptr, &shaderModule);

    Assert(result == vk::Result::eSuccess, "Failed to load shader module!", {{"Error Code", static_cast<uint32_t>(result)}});

    return shaderModule;
}

void Graphics::CreateFramebuffers()
{
    _swapChainFramebuffers.resize(_swapChainImageViews.size());

    for (size_t i = 0; i < _swapChainImageViews.size(); i++)
    {
        std::array attachments = {
            _colorImageView,
            _depthImageView,
        	_swapChainImageViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _swapChainExtent.width;
        framebufferInfo.height = _swapChainExtent.height;
        framebufferInfo.layers = 1;

        vk::Result result = _device.createFramebuffer(&framebufferInfo, nullptr, &_swapChainFramebuffers[i]);
        Assert(result == vk::Result::eSuccess, "Failed to create framebuffer!", {{"Error Code", static_cast<uint32_t>(result)}, {"index", i}});
    }
}

void Graphics::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    _framebufferResized = true;
}

void Graphics::LoadModel()
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("Data/Models/viking_room.obj", aiProcess_Triangulate | aiProcess_FlipUVs);

    Assert(scene, "Could not load model");

    for (unsigned i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* curMesh = scene->mMeshes[i];

        for (unsigned j = 0; j < curMesh->mNumVertices; ++j)
        {
            Vertex curVer{};

            curVer.pos = { curMesh->mVertices[j].x, curMesh->mVertices[j].y, curMesh->mVertices[j].z };
            curVer.color = { 1, 1, 1 };
            curVer.texCoord = { curMesh->mTextureCoords[0][j].x, curMesh->mTextureCoords[0][j].y };

            _vertices.push_back(curVer);
        }

        for (unsigned int i = 0; i < curMesh->mNumFaces; i++)
        {
            aiFace face = curMesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                _indices.push_back(face.mIndices[j]);
        }
    }
}

void Graphics::CreateVertexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();

	vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vk::Result result = _device.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlagBits{}, &data);

    if (result != vk::Result::eSuccess)
    {
        Error("Failed to map vertex memory!");
        return;
    }

    memcpy(data, _vertices.data(), bufferSize);
    _device.unmapMemory(stagingBufferMemory);
    
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _vertexBuffer,
        _vertexBufferMemory);

    CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

    _device.destroyBuffer(stagingBuffer, nullptr);
    _device.freeMemory(stagingBufferMemory, nullptr);
}

void Graphics::CreateIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
        stagingBuffer, stagingBufferMemory);

    void* data;
    vk::Result result = _device.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlagBits{}, &data);

    if (result != vk::Result::eSuccess)
    {
        Error("Failed to map index memory!");
        return;
    }

    memcpy(data, _indices.data(), bufferSize);
    _device.unmapMemory(stagingBufferMemory);

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _indexBuffer,
        _indexBufferMemory);

    CopyBuffer(stagingBuffer, _indexBuffer, bufferSize);

    _device.destroyBuffer(stagingBuffer, nullptr);
    _device.freeMemory(stagingBufferMemory, nullptr);
}

void Graphics::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(mat4) * 3;

    _uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            _uniformBuffers[i], _uniformBuffersMemory[i]);

        vk::Result result = _device.mapMemory(_uniformBuffersMemory[i], 0, bufferSize, vk::MemoryMapFlagBits{}, &_uniformBuffersMapped[i]);

        if (result != vk::Result::eSuccess)
        {
            Error("Failed to map uniform memory!");
        }
    }
}

void Graphics::CreateDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    vk::Result result = _device.createDescriptorPool(&poolInfo, nullptr, &_descriptorPool);
    Assert(result == vk::Result::eSuccess, "Failed to create descriptor pool!", { {"Error Code", static_cast<uint32_t>(result)} });
}

void Graphics::CreateDescriptorSets()
{
    vector layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    vk::Result result = _device.allocateDescriptorSets(&allocInfo, _descriptorSets.data());
    Assert(result == vk::Result::eSuccess, "Failed to sets!", { {"Error Code", static_cast<uint32_t>(result)} });

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(mat4) * 3;

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = _textureImageView;
        imageInfo.sampler = _textureSampler;

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
        
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        
        descriptorWrites[1].dstSet = _descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        _device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Graphics::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
    vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
{
    vk::BufferCreateInfo bufferInfo{};
    
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    vk::Result result = _device.createBuffer(&bufferInfo, nullptr, &buffer);
    Assert(result == vk::Result::eSuccess, "Failed to create vertex buffer!", { {"Error Code", static_cast<uint32_t>(result)} });

    vk::MemoryRequirements memRequirements;
    _device.getBufferMemoryRequirements(buffer, &memRequirements);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    result = _device.allocateMemory(&allocInfo, nullptr, &bufferMemory);
    Assert(result == vk::Result::eSuccess, "Failed to allocate memory buffer!", { {"Error Code", static_cast<uint32_t>(result)} });

    _device.bindBufferMemory(buffer, bufferMemory, 0);
}

void Graphics::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::BufferCopy copyRegion{};
    copyRegion.size = size;
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

uint32_t Graphics::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties;
    _physicalDevice.getMemoryProperties(&memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    Assert(false, "Couldn't find the right memory type!", {{"Filter", typeFilter}});

    return UINT32_MAX;
}

void Graphics::CreateCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = _queueFamilyIndices.graphicsFamily.value();

    vk::Result result = _device.createCommandPool(&poolInfo, nullptr, &_commandPool);
    Assert(result == vk::Result::eSuccess, "Failed to create command pool!", { {"Error Code", static_cast<uint32_t>(result)} });
}

void Graphics::CreateCommandBuffers()
{
    _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    vk::Result result = _device.allocateCommandBuffers(&allocInfo, _commandBuffers.data());
    Assert(result == vk::Result::eSuccess, "Failed to allocate command buffer!", { {"Error Code", static_cast<uint32_t>(result)} });
}

void Graphics::RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
    vk::CommandBufferBeginInfo beginInfo{};

    vk::Result beginResult = commandBuffer.begin(&beginInfo);
    Assert(beginResult == vk::Result::eSuccess, "Failed to begin command buffer!", { {"Error Code", static_cast<uint32_t>(beginResult)} });

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = _swapChainExtent;

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _graphicsPipeline);

    vk::Buffer vertexBuffers[] = { _vertexBuffer };
    vk::DeviceSize offsets[] = { 0 };
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

    commandBuffer.bindIndexBuffer(_indexBuffer, 0, vk::IndexType::eUint32);

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_swapChainExtent.width);
    viewport.height = static_cast<float>(_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = _swapChainExtent;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1, &_descriptorSets[currentFrame], 0, nullptr);
    commandBuffer.drawIndexed(static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();
}

vk::CommandBuffer Graphics::BeginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    vk::Result result = _device.allocateCommandBuffers(&allocInfo, &commandBuffer);

    Assert(result == vk::Result::eSuccess, "Failed to alocate command buffer!", { {"Error Code", static_cast<uint32_t>(result)} });

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    result = commandBuffer.begin(&beginInfo);
    Assert(result == vk::Result::eSuccess, "Failed to begin command buffer!", { {"Error Code", static_cast<uint32_t>(result)} });

    return commandBuffer;
}

void Graphics::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vk::Result result = _graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
    Assert(result == vk::Result::eSuccess, "Failed to end command buffer!", { {"Error Code", static_cast<uint32_t>(result)} });
    _graphicsQueue.waitIdle();
    
    _device.freeCommandBuffers(_commandPool, 1, &commandBuffer);
}

void Graphics::CreateDepthResources()
{
    vk::Format depthFormat = FindDepthFormat();

    CreateImage(_swapChainExtent.width, _swapChainExtent.height, 1, _msaaSamples,
        depthFormat, vk::ImageTiling::eOptimal, 
        vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
        _depthImage, _depthImageMemory);
    _depthImageView = CreateImageView(_depthImage, depthFormat, 1, vk::ImageAspectFlagBits::eDepth);
    TransitionImageLayout(_depthImage, depthFormat, 1,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

vk::Format Graphics::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
    vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates)
    {
        VkFormatProperties props= _physicalDevice.getFormatProperties(format);
        
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & static_cast<uint32_t>(features)) == static_cast<uint32_t>(features))
        {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & static_cast<uint32_t>(features)) == static_cast<uint32_t>(features))
        {
            return format;
        }
    }

    Assert(false, "Couldn't find a supported format");
    return vk::Format::eUndefined;
}

vk::Format Graphics::FindDepthFormat()
{
    return FindSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

bool Graphics::HasStencilComponent(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void Graphics::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("Data/Textures/viking_room.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    Assert(pixels, "Could not load texture!");

    _mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        _stagingBuffer, _stagingBufferMemory);

    void* data;
    vk::Result result = _device.mapMemory(_stagingBufferMemory, 0, imageSize, {}, &data);
    Assert(result == vk::Result::eSuccess, "Failed to map textur memory!", { {"Error Code", static_cast<uint32_t>(result)} });
    memcpy(data, pixels, imageSize);
    _device.unmapMemory(_stagingBufferMemory);

    stbi_image_free(pixels);

    CreateImage(texWidth, texHeight, _mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        _textureImage, _textureImageMemory);

    TransitionImageLayout(_textureImage, vk::Format::eR8G8B8A8Srgb, _mipLevels,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    CopyBufferToImage(_stagingBuffer, _textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    GenerateMipmaps(_textureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, _mipLevels);

    _device.destroyBuffer(_stagingBuffer, nullptr);
    _device.freeMemory(_stagingBufferMemory, nullptr);
}

void Graphics::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits samples, vk::Format format, vk::ImageTiling tiling,
    vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory)
{

    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = usage;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.flags = {}; // Optional
    imageInfo.samples = samples;

    vk::Result result = _device.createImage(&imageInfo, nullptr, &image);
    Assert(result == vk::Result::eSuccess, "Failed to create image!", { {"Error Code", static_cast<uint32_t>(result)} });

    vk::MemoryRequirements memRequirements;
    _device.getImageMemoryRequirements(image, &memRequirements);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    result = _device.allocateMemory(&allocInfo, nullptr, &imageMemory);
    Assert(result == vk::Result::eSuccess, "Failed to allocate image memory!", { {"Error Code", static_cast<uint32_t>(result)} });

    _device.bindImageMemory(image, imageMemory, 0);
}

void Graphics::TransitionImageLayout(vk::Image image, vk::Format format, uint32_t mipLevels, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = {}; // TODO
    barrier.dstAccessMask = {}; // TODO

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else
    {
       Assert(false,"unsupported layout transition!");
    }

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

        if (HasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    commandBuffer.pipelineBarrier(
        sourceStage, destinationStage,
        {},
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommands(commandBuffer);
}

void Graphics::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D{ 0, 0, 0 };
    region.imageExtent = vk::Extent3D{
        width,
        height,
        1
    };

    commandBuffer.copyBufferToImage(
        buffer,
        image,
        vk::ImageLayout::eTransferDstOptimal,
        1,
        &region
    );

    EndSingleTimeCommands(commandBuffer);
}

vk::ImageView Graphics::CreateImageView(vk::Image image, vk::Format format, uint32_t mipLevels, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vk::ImageView imageView;
    vk::Result result = _device.createImageView(&viewInfo, nullptr, &imageView);
    Assert(result == vk::Result::eSuccess , "failed to create texture image view!", { {"Error Code", static_cast<uint32_t>(result)} });

    return imageView;
}

void Graphics::CreateTextureImageView()
{
    _textureImageView = CreateImageView(_textureImage, vk::Format::eR8G8B8A8Srgb, _mipLevels, vk::ImageAspectFlagBits::eColor);
}

void Graphics::CreateTextureSampler()
{
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.minLod = 0.0f; // Optional
    samplerInfo.maxLod = static_cast<float>(_mipLevels);
    samplerInfo.mipLodBias = 0.0f; // Optional

    vk::PhysicalDeviceProperties properties{};
    _physicalDevice.getProperties(&properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;

    vk::Result result = _device.createSampler(&samplerInfo, nullptr, &_textureSampler);
    Assert(result == vk::Result::eSuccess, "failed to create texture image view!", { {"Error Code", static_cast<uint32_t>(result)} });
}

void Graphics::GenerateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	// Check if image format supports linear blitting
    vk::FormatProperties formatProperties;
    _physicalDevice.getFormatProperties(imageFormat, &formatProperties);
    Assert(static_cast<bool>((formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)),
        "texture image format does not support linear blitting!");

    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {},
            0, nullptr,
            0, nullptr,
            1, &barrier);

        vk::ImageBlit blit{};
        blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.dstOffsets[1] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        commandBuffer.blitImage(
            image, vk::ImageLayout::eTransferSrcOptimal,
            image, vk::ImageLayout::eTransferDstOptimal,
            1, &blit,
            vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
        0, nullptr,
        0, nullptr,
        1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

void Graphics::CreateSyncObjects()
{
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};

    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vk::Result imageAvailResult = _device.createSemaphore(&semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
        Assert(imageAvailResult == vk::Result::eSuccess, "Failed to create semaphore!", { {"Error Code", static_cast<uint32_t>(imageAvailResult)} });
        vk::Result renderFinResult = _device.createSemaphore(&semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
        Assert(renderFinResult == vk::Result::eSuccess, "Failed to create semaphore!", { {"Error Code", static_cast<uint32_t>(renderFinResult)} });
        vk::Result fenceResult = _device.createFence(&fenceInfo, nullptr, &_inFlightFences[i]);
        Assert(fenceResult == vk::Result::eSuccess, "Failed to create fence!", { {"Error Code", static_cast<uint32_t>(fenceResult)} });
    }

}

vk::SampleCountFlagBits Graphics::GetMaxUsableSampleCount()
{
    vk::PhysicalDeviceProperties physicalDeviceProperties;
    _physicalDevice.getProperties(&physicalDeviceProperties);

    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
    if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
    if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
    if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
    if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
    if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }

    return vk::SampleCountFlagBits::e1;
}

void Graphics::CreateColorResources()
{
    vk::Format colorFormat = _swapChainImageFormat;

    CreateImage(_swapChainExtent.width, _swapChainExtent.height, 1,
        _msaaSamples, colorFormat, vk::ImageTiling::eOptimal, 
        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _colorImage,
        _colorImageMemory);
    _colorImageView = CreateImageView(_colorImage, colorFormat, 1, vk::ImageAspectFlagBits::eColor);
}

void Graphics::DrawFrame()
{
    vk::Result result = _device.waitForFences(1, &_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    Assert(result == vk::Result::eSuccess, "Failed to wait for fence!", { {"Error Code", static_cast<uint32_t>(result)} });

    uint32_t imageIndex;
    result = _device.acquireNextImageKHR(_swapChain, UINT64_MAX, _imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == vk::Result::eErrorOutOfDateKHR)
    {
        RecreateSwapChain();

        _framebufferResized = false;

        return;
    }

    Assert(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR, "Failed to acquire swap chain image!", {{"Error code", static_cast<uint32_t>(result)}});

    result = _device.resetFences(1, &_inFlightFences[currentFrame]);
    Assert(result == vk::Result::eSuccess, "Failed to reset fence!", { {"Error Code", static_cast<uint32_t>(result)} });

    _commandBuffers[currentFrame].reset({});
    RecordCommandBuffer(_commandBuffers[currentFrame], imageIndex);

    UpdateUniformBuffer(currentFrame);

    vk::SubmitInfo submitInfo{};

    vk::Semaphore waitSemaphores[] = { _imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[currentFrame];

    vk::Semaphore signalSemaphores[] = { _renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vk::Result subResult = _graphicsQueue.submit(1, &submitInfo, _inFlightFences[currentFrame]);
    Assert(subResult == vk::Result::eSuccess, "Failed to submit draw command buffer!", { {"Error Code", static_cast<uint32_t>(subResult)} });

    vk::PresentInfoKHR presentInfo{};

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapChains[] = { _swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    result = _presentQueue.presentKHR(&presentInfo);

	if (result == vk::Result::eErrorOutOfDateKHR || _framebufferResized)
    {
        RecreateSwapChain();

        _framebufferResized = false;

        return;
    }

    Assert(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR, "Failed to acquire swap chain image!", { {"Error code", static_cast<uint32_t>(result)} });


    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Graphics::UpdateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    _model = glm::rotate(mat4(1.0f), time * radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
    _view = lookAt(vec3(2.0f, 2.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    _proj = perspective(radians(45.0f), _swapChainExtent.width / static_cast<float>(_swapChainExtent.height), 0.1f, 10.0f);
    _proj[1][1] *= -1;

    memcpy(_uniformBuffersMapped[currentImage], &_model, sizeof(mat4));
	memcpy(static_cast<char*>(_uniformBuffersMapped[currentImage]) + sizeof(mat4), &_view, sizeof(mat4));
    memcpy(static_cast<char*>(_uniformBuffersMapped[currentImage]) + sizeof(mat4) * 2, &_proj, sizeof(mat4));
}

bool Graphics::ShouldClose()
{
    return glfwWindowShouldClose(_window);
}

void Graphics::Update()
{
    glfwPollEvents();
    DrawFrame();
}

void Graphics::DeInit()
{
    //wait for the current frame to finish
    _device.waitIdle();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        _device.destroySemaphore(_renderFinishedSemaphores[i], nullptr);
        _device.destroySemaphore(_imageAvailableSemaphores[i], nullptr);
        _device.destroyFence(_inFlightFences[i], nullptr);
    }

    _device.destroyCommandPool(_commandPool, nullptr);

    CleanupSwapChain();

    _device.destroySampler(_textureSampler, nullptr);
    _device.destroyImageView(_textureImageView, nullptr);
    _device.destroyImage(_textureImage, nullptr);
    _device.freeMemory(_textureImageMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        _device.destroyBuffer(_uniformBuffers[i], nullptr);
        _device.freeMemory(_uniformBuffersMemory[i], nullptr);
    }

    _device.destroyDescriptorPool(_descriptorPool, nullptr);
	_device.destroyDescriptorSetLayout(_descriptorSetLayout, nullptr);

    _device.destroyBuffer(_indexBuffer, nullptr);
    _device.freeMemory(_indexBufferMemory, nullptr);
    _device.destroyBuffer(_vertexBuffer, nullptr);
    _device.freeMemory(_vertexBufferMemory, nullptr);

    _device.destroyPipeline(_graphicsPipeline, nullptr);
    _device.destroyPipelineLayout(_pipelineLayout, nullptr);
    _device.destroyRenderPass(_renderPass, nullptr);

    if constexpr (enableValidationLayers)
    {
        _instance.destroyDebugUtilsMessengerEXT(_debugMessenger, nullptr);
    }

    _device.destroy();
    _instance.destroySurfaceKHR(_surface, nullptr);
    _instance.destroy();
    glfwDestroyWindow(_window);

    glfwTerminate();
}