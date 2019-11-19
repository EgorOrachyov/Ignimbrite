//
// Created by Egor Orachyov on 2019-11-03.
//

#include <VulkanContext.h>
#include <renderer/Compilation.h>
#include <renderer/DeviceDefinitions.h>
#include <exception>
#include <cstring>
#include <set>
#include <array>
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
    uint32 layerCount;
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
    uint32 deviceCount = 0;
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
    uint32 extensionCount;
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
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#ifdef MODE_DEBUG
    printf("Available queue families: %u\n", queueFamilyCount);
#endif

    for (uint32 i = 0; i < queueFamilies.size(); i++) {

        VkQueueFamilyProperties& p = queueFamilies[i];

        if (p.queueCount > 0 && (p.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphicsFamily.setValue(i);
#ifdef MODE_DEBUG
            printf("Found queue family [graphics: %u]\n", i);
#endif
        }

        if (p.queueCount > 0 && (p.queueFlags & VK_QUEUE_TRANSFER_BIT)) {
            if (i != indices.graphicsFamily.get() || !indices.transferFamily.hasValue()) {
                indices.transferFamily.setValue(i);
#ifdef MODE_DEBUG
                printf("Found queue family [transfer: %u]\n", i);
#endif
            }
        }

        if (indices.isComplete()) {
            return;
        }
    }
}

void VulkanContext::findPresentsFamily(VulkanSurface &surface) {
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    uint32 presentsFamily = 0xffffffff;
    uint32 graphicsFamily = familyIndices.graphicsFamily.get();

    VkBool32 supported = 0x0;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsFamily, surface.surface, &supported);

    if (supported) {
        presentsFamily = graphicsFamily;
    } else {
        for (uint32 i = 0; i < queueFamilyCount; i++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface.surface, &supported);
            if (supported) {
                presentsFamily = i;
            }
        }
    }

    if (presentsFamily == 0xffffffff) {
        throw VulkanException("Surface does not support present queue mode");
    }

    if (presentsFamily == graphicsFamily) {
        surface.presentsFamily = presentsFamily;
        surface.presentQueue = graphicsQueue;
    } else {
        surface.presentsFamily = presentsFamily;
        vkGetDeviceQueue(device, presentsFamily, 0, &surface.presentQueue);

        if (surface.presentQueue == VK_NULL_HANDLE) {
            throw VulkanException("Failed to get present queue");
        }
    }

#ifdef MODE_DEBUG
    printf("Found queue family [present: %u]\n", presentsFamily);
#endif
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
    for (uint32 queueFamily : uniqueQueueFamilies) {
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
    bool tripleBuffering = surface.tripleBuffering;
    uint32 width = surface.widthFramebuffer;
    uint32 height = surface.heightFramebuffer;
    uint32 swapChainMinImageCount = tripleBuffering ? 3 : 2;
    VkSurfaceKHR surfaceKHR = surface.surface;
    VkSurfaceCapabilitiesKHR& surfaceCapabilities = surface.surfaceCapabilities;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
    VulkanUtils::getSurfaceProperties(physicalDevice, surfaceKHR, surfaceFormats, presentModes);

    VkSurfaceFormatKHR chosenSurfaceFormat;
    VkPresentModeKHR chosenPresentMode;

    chosenSurfaceFormat.format = surfaceFormats[0].format;
    chosenSurfaceFormat.colorSpace = surfaceFormats[0].colorSpace;

    for (auto& surfaceFormat : surfaceFormats) {
        if (surfaceFormat.format == PREFERRED_FORMAT && surfaceFormat.colorSpace == PREFERRED_COLOR_SPACE) {
            chosenSurfaceFormat.format = surfaceFormat.format;
            chosenSurfaceFormat.colorSpace = surfaceFormat.colorSpace;
            break;
        }
    }

    chosenPresentMode = presentModes[0];

    for (auto& presentMode : presentModes) {
        if (presentMode == PREFERRED_PRESENT_MODE) {
            chosenPresentMode = PREFERRED_PRESENT_MODE;
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.pNext = NULL;
    swapChainCreateInfo.surface = surfaceKHR;

    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.presentMode = chosenPresentMode;
    swapChainCreateInfo.imageFormat = chosenSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    std::array<uint32, 3> queueFamilyIndices = {
            familyIndices.graphicsFamily.get(),
            familyIndices.transferFamily.get(),
            surface.presentsFamily
    };

    // check queues, if they are from same queue families
    auto equals = true;
    auto base = queueFamilyIndices[0];
    for (auto i: queueFamilyIndices) {
        if (i != base) {
            equals = false;
            break;
        }
    }

    if (equals) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = NULL;
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    swapChainCreateInfo.imageExtent = VulkanUtils::getSwapChainExtent(width, height, surfaceCapabilities);
    swapChainCreateInfo.compositeAlpha = VulkanUtils::getAvailableCompositeAlpha(surfaceCapabilities);

    if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else {
        swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    }

    if (!(surfaceCapabilities.minImageCount <= swapChainMinImageCount
        && swapChainMinImageCount <= surfaceCapabilities.maxImageCount)) {
        throw VulkanException("Given swap chain min image count is not available on this surface and device");
    }

    swapChainCreateInfo.minImageCount = swapChainMinImageCount;
    swapChainCreateInfo.imageArrayLayers = 1;

    VkResult r = vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapChain);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create swap chain");
    }

    uint32 swapChainImageCount;
    r = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);

    if (r != VK_SUCCESS) {
        throw VulkanException("Can't get images from swap chain");
    }

    surface.swapChainImages.resize(swapChainImageCount);
    surface.swapChainImageViews.resize(swapChainImageCount);

    r = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, surface.swapChainImages.data());

    if (r != VK_SUCCESS) {
        throw VulkanException("Can't get images from swap chain");
    }

    for (uint32 i = 0; i < swapChainImageCount; i++) {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = NULL;
        viewInfo.flags = 0;

        viewInfo.image = surface.swapChainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = chosenSurfaceFormat.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        r = vkCreateImageView(device, &viewInfo, nullptr, &surface.swapChainImageViews[i]);

        if (r != VK_SUCCESS) {
            throw VulkanException("Can't create image view for swapchain");
        }
    }

    surface.swapChain = swapChain;
    surface.presentMode = chosenPresentMode;
    surface.surfaceFormat = chosenSurfaceFormat;
    surface.swapChainExtent = swapChainCreateInfo.imageExtent;
}

void VulkanContext::destroySwapChain(VulkanSurface& surface)
{
    // destroy only image views, images will be destroyed with swap chain
    for (auto view: surface.swapChainImageViews) {
        vkDestroyImageView(device, view, nullptr);
    }

    vkDestroySwapchainKHR(device, surface.swapChain, nullptr);
}

void VulkanContext::createFramebufferFormat(VulkanSurface &surface) {
    VkAttachmentDescription description = {};
    description.format = surface.surfaceFormat.format;
    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference reference = {};
    reference.attachment = 0;
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &reference;
    subpass.pDepthStencilAttachment = nullptr;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &description;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result;
    VkRenderPass renderPass;

    result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

    if (result != VK_SUCCESS) {
        throw VulkanException("Failed to create render pass for surface");
    }

    auto& format = surface.framebufferFormat;
    format.renderPass = renderPass;
    format.useDepthStencil = false;
    format.numOfAttachments = 1;
}

void VulkanContext::destroyFramebufferFormat(VulkanSurface &surface) {
    vkDestroyRenderPass(device, surface.framebufferFormat.renderPass, nullptr);
}

void VulkanContext::createFramebuffers(VulkanSurface &surface) {
    VkResult result;
    VkFramebuffer framebuffer;

    auto& imageViews = surface.swapChainImageViews;
    auto& framebuffers = surface.swapChainFramebuffers;
    framebuffers.resize(imageViews.size());

    for (uint32 i = 0; i < framebuffers.size(); i++) {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = nullptr;
        framebufferInfo.flags = 0;
        framebufferInfo.width = surface.swapChainExtent.width;
        framebufferInfo.height = surface.swapChainExtent.height;
        framebufferInfo.layers = 1;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &imageViews[i];
        framebufferInfo.renderPass = surface.framebufferFormat.renderPass;

        result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);

        if (result != VK_SUCCESS) {
            throw VulkanException("Filed to create framebuffer for surface");
        }

        framebuffers[i] = framebuffer;
    }
}

void VulkanContext::destroyFramebuffers(VulkanSurface &surface) {
    for (auto& framebuffer: surface.swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}

void VulkanContext::recreateSwapChain(VulkanSurface &surface) {
    deviceWaitIdle();

    destroyFramebuffers(surface);
    destroyFramebufferFormat(surface);
    destroySwapChain(surface);

    createSwapChain(surface);
    createFramebufferFormat(surface);
    createFramebuffers(surface);
}

void VulkanContext::deviceWaitIdle() {
    vkDeviceWaitIdle(device);
}