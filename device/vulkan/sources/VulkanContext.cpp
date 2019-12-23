/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#include <VulkanContext.h>
#include <ignimbrite/Compilation.h>
#include <ignimbrite/RenderDeviceDefinitions.h>
#include <exception>
#include <cstring>
#include <set>
#include <array>
#include <VulkanUtils.h>

#define VK_RESULT_ASSERT(result, message)                               \
    do {                                                                \
        if ((result) != VK_SUCCESS)                                     \
            throw VulkanException(message);                             \
    } while (false);

namespace ignimbrite {

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
        VK_RESULT_ASSERT(result, "Cannot create Vulkan instance");
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

    void VulkanContext::destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT inDebugMessenger,
                                                      const VkAllocationCallbacks *pAllocator) {
        auto pFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (pFunction != nullptr) {
            pFunction(instance, inDebugMessenger, pAllocator);
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

        for (auto deviceElement: devices) {
            findQueueFamilies(deviceElement, familyIndices);

            bool queueFamilySupport = familyIndices.isComplete();
            bool extensionsSupported = checkDeviceExtensionSupport(deviceElement);
            bool suitable = queueFamilySupport && extensionsSupported;

            if (suitable) {
                vkGetPhysicalDeviceFeatures(deviceElement, &deviceFeatures);
                vkGetPhysicalDeviceMemoryProperties(deviceElement, &deviceMemoryProperties);
                vkGetPhysicalDeviceProperties(deviceElement, &deviceProperties);

#ifdef MODE_DEBUG
                printf("Physical devices (count: %u). Chosen device info:\n", (uint32) devices.size());
                outDeviceInfoVerbose();
#endif
                physicalDevice = deviceElement;

                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw VulkanException("Failed to find a suitable GPU");
        }
    }


    bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice) {
        uint32 extensionCount;
        vkEnumerateDeviceExtensionProperties(inPhysicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(inPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

#ifdef MODE_DEBUG
        printf("Required (count: %u) physical device extensions:\n", (uint32) deviceExtensions.size());
        for (auto &ext: deviceExtensions) {
            printf("%s\n", ext);
        }
        printf("Available (count: %u) physical device extensions:\n", (uint32) availableExtensions.size());
        for (auto &ext: availableExtensions) {
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

    void VulkanContext::findQueueFamilies(VkPhysicalDevice inPhysicalDevice, VulkanQueueFamilyIndices &indices) {
        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(inPhysicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(inPhysicalDevice, &queueFamilyCount, queueFamilies.data());

#ifdef MODE_DEBUG
        printf("Available queue families: %u\n", queueFamilyCount);
#endif

        for (uint32 i = 0; i < queueFamilies.size(); i++) {

            VkQueueFamilyProperties &p = queueFamilies[i];

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

        surface.graphicsQueue = graphicsQueue;

#ifdef MODE_DEBUG
        printf("Found queue family [present: %u]\n", presentsFamily);
#endif
    }

    void VulkanContext::outDeviceInfoVerbose() {
        VkPhysicalDeviceProperties &properties = deviceProperties;
        VkPhysicalDeviceLimits &limits = properties.limits;

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
        std::set<uint32> uniqueQueueFamilies = {familyIndices.graphicsFamily.get(), familyIndices.transferFamily.get()};
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
            createInfo.enabledLayerCount = (uint32) validationLayers.size();
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

    void VulkanContext::createSwapChain(VulkanSurface &surface) {
        uint32 width = surface.widthFramebuffer;
        uint32 height = surface.heightFramebuffer;
        uint32 swapChainMinImageCount = SWAPCHAIN_MIN_IMAGE_COUNT;
        VkSurfaceKHR surfaceKHR = surface.surface;
        VkSurfaceCapabilitiesKHR &surfaceCapabilities = surface.surfaceCapabilities;
        VkSwapchainKHR swapChainKHR = VK_NULL_HANDLE;

        VulkanSwapChain& swapChain = surface.swapChain;

        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
        VulkanUtils::getSurfaceProperties(physicalDevice, surfaceKHR, surfaceFormats, presentModes);

        VkSurfaceFormatKHR chosenSurfaceFormat;
        VkPresentModeKHR chosenPresentMode;

        chosenSurfaceFormat.format = surfaceFormats[0].format;
        chosenSurfaceFormat.colorSpace = surfaceFormats[0].colorSpace;

        for (auto &surfaceFormat : surfaceFormats) {
            if (surfaceFormat.format == PREFERRED_FORMAT && surfaceFormat.colorSpace == PREFERRED_COLOR_SPACE) {
                chosenSurfaceFormat.format = surfaceFormat.format;
                chosenSurfaceFormat.colorSpace = surfaceFormat.colorSpace;
                break;
            }
        }

        chosenPresentMode = presentModes[0];

        for (auto &presentMode : presentModes) {
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
        } else {
            swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
        }

        if (!(swapChainMinImageCount <= surfaceCapabilities.maxImageCount || surfaceCapabilities.maxImageCount == 0)) {
            throw VulkanException("Given swap chain min image count is not available on this surface and device");
        }

        if (surfaceCapabilities.minImageCount > swapChainMinImageCount) {
            swapChainMinImageCount = surfaceCapabilities.minImageCount;
        }

        swapChainCreateInfo.minImageCount = swapChainMinImageCount;
        swapChainCreateInfo.imageArrayLayers = 1;

        VkResult result = vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapChainKHR);
        if (result != VK_SUCCESS) {
            throw VulkanException("Can't create swap chain");
        }

        uint32 swapChainImageCount;
        result = vkGetSwapchainImagesKHR(device, swapChainKHR, &swapChainImageCount, nullptr);

        if (result != VK_SUCCESS) {
            throw VulkanException("Can't get images from swap chain");
        }

        swapChain.images.resize(swapChainImageCount);
        swapChain.imageViews.resize(swapChainImageCount);

        result = vkGetSwapchainImagesKHR(device, swapChainKHR, &swapChainImageCount, swapChain.images.data());

        if (result != VK_SUCCESS) {
            throw VulkanException("Can't get images from swap chain");
        }

        for (uint32 i = 0; i < swapChainImageCount; i++) {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;

            viewInfo.image = swapChain.images[i];
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

            result = vkCreateImageView(device, &viewInfo, nullptr, &swapChain.imageViews[i]);

            if (result != VK_SUCCESS) {
                throw VulkanException("Can't create image view for swapchain");
            }
        }

        // Setup depth stencil buffers for each swap chain image
        swapChain.depthStencilImages.resize(swapChainImageCount);
        swapChain.depthStencilImageViews.resize(swapChainImageCount);
        swapChain.depthStencilImageMemory.resize(swapChainImageCount);

        VkFormat depthFormats[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;
        VkFormatFeatureFlags featureFlags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        VkFormat depthFormat = VulkanUtils::findSupportedFormat(*this, depthFormats, 2, imageTiling, featureFlags);

        swapChain.depthFormat = depthFormat;

        for (uint32 i = 0; i < swapChainImageCount; i++) {
            VulkanUtils::createImage(
                    *this,
                    width, height, 1, 1,
                    VK_IMAGE_TYPE_2D, depthFormat,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    swapChain.depthStencilImages[i],
                    swapChain.depthStencilImageMemory[i]
             );

            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            VkComponentMapping components = {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY
            };

            VulkanUtils::createImageView(
                    *this,
                    swapChain.depthStencilImageViews[i],
                    swapChain.depthStencilImages[i],
                    VK_IMAGE_VIEW_TYPE_2D,
                    depthFormat,
                    subresourceRange,
                    components
            );
        }

        // create semaphores for each image
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        surface.imageAvailableSemaphores.resize(surface.maxFramesInFlight);
        surface.renderFinishedSemaphores.resize(surface.maxFramesInFlight);
        surface.inFlightFences.resize(surface.maxFramesInFlight);
        surface.imagesInFlight.resize(swapChainImageCount);

        for (uint32 i = 0; i < surface.maxFramesInFlight; i++) {
            result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &surface.imageAvailableSemaphores[i]);
            if (result != VK_SUCCESS) {
                throw VulkanException("Can't create semaphore");
            }

            result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &surface.renderFinishedSemaphores[i]);
            if (result != VK_SUCCESS) {
                throw VulkanException("Can't create semaphore");
            }

            result = vkCreateFence(device, &fenceCreateInfo, nullptr, &surface.inFlightFences[i]);
            if (result != VK_SUCCESS) {
                throw VulkanException("Can't create fence");
            }
        }

        for (uint32 i = 0; i < swapChainImageCount; i++) {
            surface.imagesInFlight[i] = VK_NULL_HANDLE;
        }

        surface.presentMode = chosenPresentMode;
        surface.surfaceFormat = chosenSurfaceFormat;
        swapChain.extent = swapChainCreateInfo.imageExtent;
        swapChain.swapChainKHR = swapChainKHR;
    }

    void VulkanContext::destroySwapChain(VulkanSurface &surface) {
        for (VkSemaphore &semaphore : surface.renderFinishedSemaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }

        for (VkSemaphore &semaphore : surface.imageAvailableSemaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }

        for (VkFence &fence : surface.inFlightFences) {
            vkDestroyFence(device, fence, nullptr);
        }

        // Counts of images for all image related objects are equal
        VulkanSwapChain& swapChain = surface.swapChain;
        uint32 swapChainObjects = swapChain.images.size();

        for (uint32 i = 0; i < swapChainObjects; i++) {
            // destroy only image views, images will be destroyed with swap chain
            vkDestroyImageView(device, swapChain.imageViews[i], nullptr);
            // destroy manually created depth stencil buffers
            vkDestroyImageView(device, swapChain.depthStencilImageViews[i], nullptr);
            vkDestroyImage(device, swapChain.depthStencilImages[i], nullptr);
            vkFreeMemory(device, swapChain.depthStencilImageMemory[i], nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain.swapChainKHR, nullptr);
    }

    void VulkanContext::recreateSwapChain(VulkanSurface &surface) {
        deviceWaitIdle();

        destroyCommandBuffers(surface);
        destroyFramebuffers(surface);
        destroyFramebufferFormat(surface);
        destroySwapChain(surface);

        createSwapChain(surface);
        createFramebufferFormat(surface);
        createFramebuffers(surface);
        createCommandBuffers(surface);
    }

    void VulkanContext::createFramebufferFormat(VulkanSurface &surface) {
        VkAttachmentDescription descriptions[2] = {};

        descriptions[0].format = surface.surfaceFormat.format;
        descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        descriptions[1].format = surface.swapChain.depthFormat;
        descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference references[2] = {};

        references[0].attachment = 0;
        references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        references[1].attachment = 1;
        references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
        subpass.pColorAttachments = &references[0];
        subpass.pDepthStencilAttachment = &references[1];

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = descriptions;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult result;
        VkRenderPass renderPass;

        result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

        VK_RESULT_ASSERT(result, "Failed to create render pass for surface");

        auto &format = surface.swapChain.framebufferFormat;
        format.renderPass = renderPass;
        format.useDepthStencil = true;
        format.numOfAttachments = 2;
    }

    void VulkanContext::destroyFramebufferFormat(VulkanSurface &surface) {
        vkDestroyRenderPass(device, surface.swapChain.framebufferFormat.renderPass, nullptr);
    }

    void VulkanContext::createFramebuffers(VulkanSurface &surface) {
        VkResult result;

        auto &swapChain = surface.swapChain;
        auto &framebuffers = swapChain.framebuffers;
        framebuffers.resize(swapChain.imageViews.size());

        for (size_t i = 0; i < framebuffers.size(); i++) {
            VkImageView imageViews[2] = { swapChain.imageViews[i], swapChain.depthStencilImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = nullptr;
            framebufferInfo.flags = 0;
            framebufferInfo.width = swapChain.extent.width;
            framebufferInfo.height = swapChain.extent.height;
            framebufferInfo.layers = 1;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = imageViews;
            framebufferInfo.renderPass = swapChain.framebufferFormat.renderPass;

            result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);

            VK_RESULT_ASSERT(result, "Filed to create framebuffer for surface");
        }
    }

    void VulkanContext::destroyFramebuffers(VulkanSurface &surface) {
        for (auto &framebuffer: surface.swapChain.framebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    }

    void VulkanContext::deviceWaitIdle() {
        vkDeviceWaitIdle(device);
    }

    void VulkanContext::createCommandPools() {
        graphicsCommandPool = VulkanUtils::createCommandPool(*this,
                                                             VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                             familyIndices.graphicsFamily.get());
        transferCommandPool = VulkanUtils::createCommandPool(*this,
                                                             VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                             familyIndices.transferFamily.get());

        graphicsTempCommandPool = VulkanUtils::createCommandPool(*this,
                                                                 VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                                                 familyIndices.graphicsFamily.get());
        transferTempCommandPool = VulkanUtils::createCommandPool(*this,
                                                                 VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                                                 familyIndices.transferFamily.get());
    }

    void VulkanContext::destroyCommandPools() {
        vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
        vkDestroyCommandPool(device, transferCommandPool, nullptr);
        vkDestroyCommandPool(device, graphicsTempCommandPool, nullptr);
        vkDestroyCommandPool(device, transferTempCommandPool, nullptr);
    }

    void VulkanContext::createCommandBuffers(VulkanSurface &surface) {
//    // create command buffers for each swapchain image
//    std::vector<VkCommandBuffer> &cmdBuffers = surface.drawCmdBuffers;
//    cmdBuffers.resize(surface.tripleBuffering ? 3 : 2);
//
//    VkCommandBufferAllocateInfo cmdBufferInfo {};
//    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//    cmdBufferInfo.commandPool = this->graphicsCommandPool;
//    cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//    cmdBufferInfo.commandBufferCount = cmdBuffers.size();
//
//    VkResult r = vkAllocateCommandBuffers(device, &cmdBufferInfo, cmdBuffers.data());
//
//    if (r != VK_SUCCESS) {
//        throw VulkanException("Can't allocate command buffers for swapchain");
//    }
    }

    void VulkanContext::destroyCommandBuffers(VulkanSurface &surface) {
//    vkFreeCommandBuffers(device, this->graphicsCommandPool, surface.drawCmdBuffers.size(), surface.drawCmdBuffers.data());
    }

} // namespace ignimbrite
