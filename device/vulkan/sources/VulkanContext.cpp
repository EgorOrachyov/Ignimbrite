//
// Created by Egor Orachyov on 2019-11-03.
//

#include <VulkanContext.h>
#include <renderer/Compilation.h>
#include <renderer/DeviceDefinitions.h>
#include <exception>
#include <cstring>
#include <set>
#include "VulkanUtils.h"

void VulkanContext::createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "default";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "default";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = (uint32) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if (enableValidationLayers) {
        if (checkValidationLayers()) {
            createInfo.enabledLayerCount = (uint32) validationLayers.size();
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            throw VulkanException("Required validation layer is not available");
        }
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    checkSupportedExtensions();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
        throw VulkanException("Cannot create Vulkan instance");
    }
}

void VulkanContext::destroyInstance() {
    vkDestroyInstance(instance, nullptr);
}

void VulkanContext::fillRequiredExt(uint32 count, const char *const *ext) {
    if (count > 0) {
        requiredExtensions.reserve(count + 1);
        for (uint32 i = 0; i < count; i++) {
            requiredExtensions.push_back(ext[i]);
        }
    }

    if (enableValidationLayers) {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
}

void VulkanContext::checkSupportedExtensions() {
    uint32 extensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

#ifdef MODE_DEBUG
    printf("Required (count: %u) extensions for vulkan:\n", (uint32) requiredExtensions.size());
    for (const auto &e: requiredExtensions) {
        printf("%s\n", e);
    }

    printf("Supported (count: %u) extensions by vulkan:\n", (uint32) extensions.size());
    for (const auto &e: extensions) {
        printf("%s\n", e.extensionName);
    }
#endif
}

bool VulkanContext::checkValidationLayers() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

#ifdef MODE_DEBUG
    printf("Required (count: %u) validation layers for vulkan:\n", (uint32) validationLayers.size());
    for (const auto &required: validationLayers) {
        printf("%s\n", required);
    }

    printf("Available (count: %u) validation layers by vulkan:\n", layerCount);
    for (const auto &available: availableLayers) {
        printf("%s\n", available.layerName);
    }
#endif

    for (const auto &required: validationLayers) {
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

void VulkanContext::setupDebugMessenger() {
    if (!enableValidationLayers) {
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
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = this; // Optional

    if (createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw VulkanException("Failed to create debug utils messenger");
    }
}

void VulkanContext::destroyDebugMessenger() {
    if (!enableValidationLayers) {
        return;
    }

    destroyDebugUtilsMessengerEXT(debugMessenger, nullptr);
}

VkResult VulkanContext::createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
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

void VulkanContext::destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
                                                   const VkAllocationCallbacks *pAllocator) {
    auto pFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (pFunction != nullptr) {
        pFunction(instance, debugMessenger, pAllocator);
    } else {
        throw VulkanException("Cannot load \"vkDestroyDebugUtilsMessengerEXT\" function");
    }
}

VkBool32 VulkanContext::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                       void *pUserData) {
    printf("[Vk Validation layer]: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

void VulkanContext::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw VulkanException("No target GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (auto device: devices) {
        findQueueFamilies(device, familyIndices);

        bool queueFamilySupport = familyIndices.isComplete();
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool suitable = queueFamilySupport && extensionsSupported;

        if (suitable) {
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

#ifdef MODE_DEBUG
            printf("Physical devices (count: %u). Chosen device info:\n", (uint32) devices.size());
            outDeviceInfoVerbose();
#endif
            physicalDevice = device;

            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw VulkanException("Failed to find a suitable GPU");
    }
}


bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

#ifdef MODE_DEBUG
    printf("Required (count: %u) physical device extensions:\n", (uint32) deviceExtensions.size());
    for (auto& ext: deviceExtensions) {
        printf("%s\n", ext);
    }
    printf("Available (count: %u) physical device extensions:\n", (uint32) availableExtensions.size());
    for (auto& ext: availableExtensions) {
        printf("%s\n", ext.extensionName);
    }
#endif

    for (auto &required: deviceExtensions) {
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

void VulkanContext::findQueueFamilies(VkPhysicalDevice device, VulkanQueueFamilyIndices &indices) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32 i = 0; i < queueFamilies.size(); i++) {
        VkQueueFamilyProperties& p = queueFamilies[i];

        if (p.queueCount > 0 && (p.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphicsFamily.setValue(i);
        }

        if (p.queueCount > 0 && (p.queueFlags & VK_QUEUE_TRANSFER_BIT)) {
            if (i != indices.graphicsFamily.get() || !indices.transferFamily.hasValue()) {
                indices.transferFamily.setValue(i);
            }
        }

        if (indices.isComplete()) {
            return;
        }
    }
}

void VulkanContext::outDeviceInfoVerbose() {
    VkPhysicalDeviceProperties& properties = deviceProperties;
    VkPhysicalDeviceLimits& limits = properties.limits;

    printf("Name: %s\n", properties.deviceName);
    printf("Device ID: %x\n", properties.deviceID);
    printf("Vendor ID: %x\n", properties.vendorID);
    printf("API version: %x\n", properties.apiVersion);
    printf("Driver version: %x\n", properties.driverVersion);

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

void VulkanContext::createLogicalDevice() {
    /** Want to create only one instance of the same queue */
    std::set<uint32> uniqueQueueFamilies = { familyIndices.graphicsFamily.get(), familyIndices.transferFamily.get() };
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

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32) queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = (uint32) deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = (uint32 )validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw VulkanException("Failed to create logical device");
    }

    vkGetDeviceQueue(device, familyIndices.graphicsFamily.get(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, familyIndices.transferFamily.get(), 0, &transferQueue);
}

void VulkanContext::destroyLogicalDevice() {
    vkDestroyDevice(device, nullptr);
}

void VulkanContext::createSwapChain(VulkanSurface& surface) {
    const bool tripleBuffering = surface.tripleBuffering;
    const VkPresentModeKHR preferredPresentMode = surface.preferredPresentMode;
    const VkFormat preferredSurfFormat = surface.preferredSurfFormat;
    const VkColorSpaceKHR preferredColorSpace = surface.preferredColorSpace;

    const uint32_t preferredWidth = surface.width;
    const uint32_t preferredHeight = surface.height;
    const uint32_t swapchainMinImageCount = tripleBuffering ? 3 : 2;
    const VkSurfaceCapabilitiesKHR& surfCapabilities = surface.surfaceCapabilities;

    std::vector<VkSurfaceFormatKHR> surfFormats;
    std::vector<VkPresentModeKHR> presentModes;

    VkSurfaceFormatKHR choosedSurfFormat;
    VkPresentModeKHR choosedPresentMode;

    VulkanUtils::getSurfaceProperties(physicalDevice, surface.surface, surfFormats, presentModes);

    // choose surface format
    choosedSurfFormat.format = surfFormats[0].format;
    choosedSurfFormat.colorSpace = surfFormats[0].colorSpace;

    for (auto& surfFormat : surfFormats) {
        if (surfFormat.format == preferredSurfFormat && surfFormat.colorSpace == preferredColorSpace) {
            choosedSurfFormat.format = surfFormat.format;
            choosedSurfFormat.colorSpace = surfFormat.colorSpace;

            break;
        }
    }

    // choose present mode
    choosedPresentMode = presentModes[0];

    for (auto& presentMode : presentModes) {
        if (presentMode == preferredPresentMode) {
            choosedPresentMode = preferredPresentMode;
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = NULL;
    swapchainCreateInfo.surface = surface.surface;

    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.presentMode = choosedPresentMode;
    swapchainCreateInfo.imageFormat = choosedSurfFormat.format;
    swapchainCreateInfo.imageColorSpace = choosedSurfFormat.colorSpace;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t graphicsQueueFamilyIndex = familyIndices.graphicsFamily.get();
    // TODO: init present queue family index in findQueueFamilies
    // using vkGetPhysicalDeviceSurfaceSupportKHR(..)
    uint32_t presentQueueFamilyIndex = 0;
    // remove this, when "presentQueueFamilyIndex" will be initialized
    throw VulkanException("presentQueueFamilyIndex is not presented");

    std::vector<uint32_t> queueFamilyIndices = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };

    // check queues, if they are from same queue families
    if (graphicsQueueFamilyIndex == presentQueueFamilyIndex) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // this parameters will be ignored as not concurrent sharing mode
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = NULL;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    swapchainCreateInfo.imageExtent = VulkanUtils::getSwaphainExtent(preferredWidth, preferredHeight, surfCapabilities);
    swapchainCreateInfo.compositeAlpha = VulkanUtils::getAvailableCompositeAlpha(surfCapabilities);

    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else {
        swapchainCreateInfo.preTransform = surfCapabilities.currentTransform;
    }

    if (!(surfCapabilities.minImageCount <= swapchainMinImageCount 
        && swapchainMinImageCount <= surfCapabilities.maxImageCount)) {
        throw VulkanException("Given swapchainMinImageCount is not available on this surface and device");
    }

    swapchainCreateInfo.minImageCount = swapchainMinImageCount;
    swapchainCreateInfo.imageArrayLayers = 1;

    // create swapchain and images that associated with it
    VkResult r = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create swapchain");
    }

    // get actual amount of images in swapchain
    uint32_t swapchainImageCount;
    VkResult r = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);

    if (r != VK_SUCCESS) {
        throw VulkanException("Can't get images from swapchain");
    }

    // allocate buffer memory
    swapchainBuffers.resize(swapchainImageCount);

    // get images themselves
    std::vector<VkImage> swapchainImages(swapchainImageCount);
    r = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());

    if (r != VK_SUCCESS) {
        throw VulkanException("Can't get images from swapchain");
    }

    // create image view for each image in swapchain
    for (uint32_t i = 0; i < swapchainImageCount; i++) {
        swapchainBuffers[i].image = swapchainImages[i];

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = NULL;
        viewInfo.flags = 0;

        viewInfo.image = swapchainBuffers[i].image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = choosedSurfFormat.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        r = vkCreateImageView(device, &viewInfo, nullptr, &swapchainBuffers[i].imageView);

        if (r != VK_SUCCESS) {
            throw VulkanException("Can't create image view for swapchain");
        }
    }
}

void VulkanContext::destroySwapChain()
{
    // destroy only image views, images will be destroyed with swapchain
    for (uint32_t i = 0; i < swapchainBuffers.size(); i++) {
        vkDestroyImageView(device, swapchainBuffers[i].imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}