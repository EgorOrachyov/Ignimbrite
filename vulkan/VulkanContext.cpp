//
// Created by Egor Orachyov on 2019-10-15.
//

#include "VulkanContext.h"
#include <Platform.h>
#include <set>

VulkanContext::VulkanContext(VulkanApplication &app)
        : mEnableValidationLayers(app.enableValidation),
          mApp(app),
          mWindow(app.getPrimaryWindow()) {
    _fillRequiredExt(app.extensionsCount, app.extensions);
    _createInstance();
    _setupDebugMessenger();
    _createSurface();
    _pickPhysicalDevice();
    _createLogicalDevice();
    _setupQueue();
    _createSwapChain();
    _createImageViews(mWindow);
}

VulkanContext::~VulkanContext() {
    _destroyImageViews(mWindow);
    _destroySwapChain();
    _destroyLogicalDevice();
    _destroySurface();
    _destroyDebugMessenger();
    _destroyInstance();
}

void VulkanContext::_createInstance() {
    /** General application info */
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = mApp.name.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = (uint32) mRequiredExtensions.size();
    createInfo.ppEnabledExtensionNames = mRequiredExtensions.data();

    /** Validation layers check*/
    if (mEnableValidationLayers) {
        if (_checkValidationLayers()) {
            createInfo.enabledLayerCount = (uint32) mValidationLayers.size();
            createInfo.ppEnabledLayerNames = mValidationLayers.data();
        } else {
            throw std::runtime_error("Required validation layer is not available");
        }
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    /** Debug extensions info check and output */
    _checkSupportedExtensions();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("Cannot create VK instance");
    }
}

void VulkanContext::_destroyInstance() {
    vkDestroyInstance(mInstance, nullptr);
}

void VulkanContext::_checkSupportedExtensions() {
    uint32 extensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

#ifdef MODE_DEBUG
    printf("Required (count: %u) extensions for vulkan:\n", (uint32) mRequiredExtensions.size());
    for (const auto &e: mRequiredExtensions) {
        printf("%s\n", e);
    }

    printf("Supported (count: %u) extensions by vulkan:\n", (uint32) extensions.size());
    for (const auto &e: extensions) {
        printf("%s\n", e.extensionName);
    }
#endif
}

bool VulkanContext::_checkValidationLayers() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

#ifdef MODE_DEBUG
    printf("Required (count: %u) validation layers for vulkan:\n", (uint32) mValidationLayers.size());
    for (const auto &required: mValidationLayers) {
        printf("%s\n", required);
    }

    printf("Available (count: %u) validation layers by vulkan:\n", layerCount);
    for (const auto &available: availableLayers) {
        printf("%s\n", available.layerName);
    }
#endif

    for (const auto &required: mValidationLayers) {
        bool found = false;
        for (const auto &available: availableLayers) {
            if (std::strcmp(required, available.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

void VulkanContext::_fillRequiredExt(uint32 count, const char *const *ext) {
    if (count > 0) {
        mRequiredExtensions.reserve(count + 1);
        for (uint32 i = 0; i < count; i++) {
            mRequiredExtensions.push_back(ext[i]);
        }
    }

    if (mEnableValidationLayers) {
        mRequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
}

void VulkanContext::_setupDebugMessenger() {
    if (!mEnableValidationLayers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = _debugCallback;
    createInfo.pUserData = this; // Optional

    if (_createDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create debug utils messenger");
    }
}

void VulkanContext::_destroyDebugMessenger() {
    if (!mEnableValidationLayers) {
        return;
    }

    _destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
}

VkResult VulkanContext::_createDebugUtilsMessengerEXT(VkInstance instance,
                                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator,
                                                      VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto pFunction = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (pFunction != nullptr) {
        return pFunction(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanContext::_destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                   const VkAllocationCallbacks *pAllocator) {
    auto pFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (pFunction != nullptr) {
        pFunction(instance, debugMessenger, pAllocator);
    } else {
        throw std::runtime_error("Cannot load \"vkDestroyDebugUtilsMessengerEXT\" function");
    }
}

VkBool32 VulkanContext::_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                       void *pUserData) {
    printf("[Vk Validation layer]: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

void VulkanContext::_pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("No target GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

#ifdef MODE_DEBUG
    printf("Physical devices (count: %u) info:\n", (uint32) devices.size());
    for (auto &device: devices) {
        _outDeviceInfoVerbose(device);
    }
#endif

    for (auto &device: devices) {
        if (_isDeviceSuitable(device)) {
            mPhysicalDevice = device;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error("Failed to find a suitable GPU");
    }
}

bool VulkanContext::_isDeviceSuitable(VkPhysicalDevice device) {
    VulkanQueueFamilyIndices indices;
    _findQueueFamilies(device, indices);

    auto queueFamilySupport = indices.isComplete();
    auto extensionsSupported = _checkDeviceExtensionSupport(device);
    auto swapChainAdequate = false;

    if (extensionsSupported) {
        VulkanSwapChainSupportDetails details;
        _querySwapChainSupport(device, details);
        swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
    }

    return queueFamilySupport && extensionsSupported && swapChainAdequate;
}

bool VulkanContext::_checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

#ifdef MODE_DEBUG
    printf("Required (count: %u) physical device extensions:\n", (uint32) mDeviceExtensions.size());
    for (auto& ext: mDeviceExtensions) {
        printf("%s\n", ext);
    }
    printf("Available (count: %u) physical device extensions:\n", (uint32) availableExtensions.size());
    for (auto& ext: availableExtensions) {
        printf("%s\n", ext.extensionName);
    }
#endif

    for (auto &required: mDeviceExtensions) {
        bool found = false;
        for (auto &available: availableExtensions) {
            if (std::strcmp(required, available.extensionName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

void VulkanContext::_querySwapChainSupport(VkPhysicalDevice device, VulkanSwapChainSupportDetails &details) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mWindow.surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mWindow.surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mWindow.surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mWindow.surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, mWindow.surface, &presentModeCount, details.presentModes.data());
    }
}

void VulkanContext::_findQueueFamilies(VkPhysicalDevice device, VulkanQueueFamilyIndices &indices) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32 i = 0; i < queueFamilies.size(); i++) {
        VkQueueFamilyProperties& p = queueFamilies[i];

        if (p.queueCount > 0 && (p.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphicsFamily.setValue(i);
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mWindow.surface, &presentSupport);

        if (p.queueCount > 0 && presentSupport) {
            indices.presentFamily.setValue(i);
        }

        if (indices.isComplete()) {
            return;
        }
    }
}

void VulkanContext::_outDeviceInfoVerbose(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    VkPhysicalDeviceLimits& limits = deviceProperties.limits;

    printf("Name: %s\n", deviceProperties.deviceName);
    printf("Device ID: %u\n", deviceProperties.deviceID);
    printf("Vendor ID: %u\n", deviceProperties.vendorID);
    printf("API version: %u\n", deviceProperties.apiVersion);
    printf("Driver version: %u\n", deviceProperties.driverVersion);

    printf("maxImageDimension1D = %u\n", limits.maxImageDimension1D);
    printf("maxImageDimension2D = %u\n", limits.maxImageDimension2D);
    printf("maxImageDimension3D = %u\n", limits.maxImageDimension3D);
    printf("maxImageDimensionCube = %u\n", limits.maxImageDimensionCube);

    printf("maxUniformBufferRange = %u\n", limits.maxUniformBufferRange);
    printf("maxMemoryAllocationCount = %u\n", limits.maxMemoryAllocationCount);
    printf("maxSamplerAllocationCount = %u\n", limits.maxSamplerAllocationCount);

    printf("maxPerStageDescriptorSamplers = %u\n", limits.maxPerStageDescriptorSamplers);
    printf("maxPerStageDescriptorUniformBuffers = %u\n", limits.maxPerStageDescriptorUniformBuffers);
    printf("maxPerStageDescriptorStorageBuffers = %u\n", limits.maxPerStageDescriptorStorageBuffers);
    printf("maxPerStageDescriptorSampledImages = %u\n", limits.maxPerStageDescriptorSampledImages);
    printf("maxPerStageDescriptorStorageImages = %u\n", limits.maxPerStageDescriptorStorageImages);
    printf("maxPerStageDescriptorInputAttachments = %u\n", limits.maxPerStageDescriptorInputAttachments);
    printf("maxPerStageResources = %u\n", limits.maxPerStageResources);

    printf("maxVertexInputAttributes = %u\n", limits.maxVertexInputAttributes);
    printf("maxVertexInputBindings = %u\n", limits.maxVertexInputBindings);
    printf("maxVertexInputAttributeOffset = %u\n", limits.maxVertexInputAttributeOffset);
    printf("maxVertexInputBindingStride = %u\n", limits.maxVertexInputBindingStride);
    printf("maxVertexOutputComponents = %u\n", limits.maxVertexOutputComponents);

    printf("maxFragmentInputComponents = %u\n", limits.maxFragmentInputComponents);
    printf("maxFragmentOutputAttachments = %u\n", limits.maxFragmentOutputAttachments);
    printf("maxFragmentDualSrcAttachments = %u\n", limits.maxFragmentDualSrcAttachments);
    printf("maxFragmentCombinedOutputResources = %u\n", limits.maxFragmentCombinedOutputResources);

}

void VulkanContext::_createLogicalDevice() {
    /** Fill data about the needed queues */
    _findQueueFamilies(mPhysicalDevice, mIndices);

    /** Want to create only one instance of the same queue */
    std::set<uint32> uniqueQueueFamilies = { mIndices.graphicsFamily.get(), mIndices.presentFamily.get() };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32) queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = (uint32) mDeviceExtensions.size();
    createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

    if (mEnableValidationLayers) {
        createInfo.enabledLayerCount = (uint32 )mValidationLayers.size();
        createInfo.ppEnabledLayerNames = mValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }
}

void VulkanContext::_destroyLogicalDevice() {
    vkDestroyDevice(mDevice, nullptr);
}

void VulkanContext::_setupQueue() {
    vkGetDeviceQueue(mDevice, mIndices.graphicsFamily.get(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, mIndices.presentFamily.get(), 0, &mPresentQueue);
}

void VulkanContext::_createSurface() {
#ifdef WSI_GLFW
    VkResult result = glfwCreateWindowSurface(mInstance, mWindow.handle, nullptr, &mWindow.surface);
#endif
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
}

void VulkanContext::_destroySurface() {
    vkDestroySurfaceKHR(mInstance, mWindow.surface, nullptr);
}

VkSurfaceFormatKHR VulkanContext::_chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == format &&
            availableFormat.colorSpace == colorSpace) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanContext::_chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    VkPresentModeKHR best = VK_PRESENT_MODE_MAILBOX_KHR;
    VkPresentModeKHR good = VK_PRESENT_MODE_FIFO_KHR;
    VkPresentModeKHR available = VK_PRESENT_MODE_IMMEDIATE_KHR;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == best) {
            return availablePresentMode;
        } else if (availablePresentMode == good) {
            available = good;
        }
    }

    return available;
}

VkExtent2D VulkanContext::_chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = { mWindow.width, mWindow.height };

        actualExtent.width =
                std::max(capabilities.minImageExtent.width,
                        std::min(capabilities.maxImageExtent.width, actualExtent.width));

        actualExtent.height =
                std::max(capabilities.minImageExtent.height,
                        std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

void VulkanContext::_createSwapChain() {
    VulkanSwapChainSupportDetails details;
    _querySwapChainSupport(mPhysicalDevice, details);

    VkSurfaceFormatKHR surfaceFormat = _chooseSwapSurfaceFormat(details.formats);
    VkPresentModeKHR presentMode = _chooseSwapPresentMode(details.presentModes);
    VkExtent2D extent = _chooseSwapExtent(details.capabilities);
    uint32 imageCount = details.capabilities.minImageCount + 1;

    if (details.capabilities.maxImageCount > 0 &&
        imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mWindow.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32 graphicsFamily = mIndices.graphicsFamily.get();
    uint32 presentFamily = mIndices.presentFamily.get();

    uint32_t queueFamilyIndices[] = {
            graphicsFamily,
            presentFamily
    };

    if (graphicsFamily != presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mWindow.swapChain)) {
        throw std::runtime_error("Failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(mDevice, mWindow.swapChain, &imageCount, nullptr);
    mWindow.swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mWindow.swapChain, &imageCount, mWindow.swapChainImages.data());

    mWindow.swapChainImageFormat = surfaceFormat.format;
    mWindow.swapChainExtent = extent;
}

void VulkanContext::_destroySwapChain() {
    vkDestroySwapchainKHR(mDevice, mWindow.swapChain, nullptr);
}

void VulkanContext::_createImageViews(VulkanWindow &window) {
    window.swapChainImageViews.resize(window.swapChainImages.size());

    for (uint32 i = 0; i < window.swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = window.swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = window.swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(mDevice, &createInfo, nullptr, &window.swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void VulkanContext::_destroyImageViews(VulkanWindow &window) {
    for (auto &imageView: window.swapChainImageViews) {
        vkDestroyImageView(mDevice, imageView, nullptr);
    }
}


