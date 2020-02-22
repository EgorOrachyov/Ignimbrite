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
#include <cstring>
#include <set>
#include <array>
#include <VulkanUtils.h>

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

    void VulkanContext::createCommandPools() {
        graphicsCommandPool = VulkanUtils::createCommandPool(
                                                             VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                             familyIndices.graphicsFamily.get());
        transferCommandPool = VulkanUtils::createCommandPool(
                                                             VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                             familyIndices.transferFamily.get());

        graphicsTmpCommandPool = VulkanUtils::createCommandPool(
                                                                 VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                                                 familyIndices.graphicsFamily.get());
        transferTmpCommandPool = VulkanUtils::createCommandPool(
                                                                 VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                                                 familyIndices.transferFamily.get());
    }

    void VulkanContext::destroyCommandPools() {
        vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
        vkDestroyCommandPool(device, transferCommandPool, nullptr);
        vkDestroyCommandPool(device, graphicsTmpCommandPool, nullptr);
        vkDestroyCommandPool(device, transferTmpCommandPool, nullptr);
    }

    void VulkanContext::deviceWaitIdle() {
        VkResult result = vkDeviceWaitIdle(device);
        VK_RESULT_ASSERT(result, "Failed to wait idle on device")
    }

    VulkanContext& VulkanContext::getInstance() {
        static VulkanContext context;
        return context;
    }

    void VulkanContext::createAllocator() {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;

        VkResult result = vmaCreateAllocator(&allocatorInfo, &vmAllocator);
        VK_RESULT_ASSERT(result,"Failed to create Vulkan memory allocator");
    }

    void VulkanContext::destroyAllocator() {
        vmaDestroyAllocator(vmAllocator);
    }


} // namespace ignimbrite
