//
// Created by Egor Orachyov on 2019-11-03.
//

#include <VulkanContext.h>
#include <renderer/Compilation.h>
#include <renderer/DeviceDefinitions.h>
#include <exception>
#include <cstring>
#include <set>

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

void VulkanContext::findQueueFamilies(VkPhysicalDevice device, QueueFamilyIndices &indices) {
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

VkFormatProperties VulkanContext::getDeviceFormatProperties(VkFormat format) const {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

    return properties;
}

uint32_t VulkanContext::getMemoryTypeIndex(uint32_t memoryTypeBits, VkFlags requirementsMask) const {
    // for each memory type available for this device
    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
        // if type is available
        if ((memoryTypeBits & 1) == 1) {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
                return i;
            }
        }

        memoryTypeBits >>= 1;
    }

    throw VulkanException("Can't find memory type in device memory properties");
}

void VulkanContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                 VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = (VkDeviceSize) size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult r = vkCreateBuffer(device, &bufferInfo, nullptr, &outBuffer);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create buffer for vertex data");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, outBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

    r = vkAllocateMemory(device, &allocInfo, nullptr, &outBufferMemory);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't allocate memory for vertex buffer");
    }

    r = vkBindBufferMemory(device, outBuffer, outBufferMemory, 0);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't bind buffer memory for vertex buffer");
    }
}

void VulkanContext::createBufferLocal(const void *data, VkDeviceSize size, VkBufferUsageFlags usage,
                                      VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    updateBufferMemory(stagingBufferMemory, 0, size, data);

    createBuffer(size,
                 usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 outBuffer,
                 outBufferMemory);

    copyBuffer(commandPool, transferQueue, stagingBuffer, outBuffer, size);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanContext::copyBuffer(VkCommandPool commandPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer,
                               VkDeviceSize size) {
    // TODO: create _beginTempCommandBuffer and _endTempCommandBuffer
    VkCommandBuffer commandBuffer; // = _beginTempCommandBuffer(device, commandPool);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    copyRegion.dstOffset = 0;
    copyRegion.srcOffset = 0;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void VulkanContext::updateBufferMemory(VkDeviceMemory bufferMemory, VkDeviceSize offset, VkDeviceSize size,
                                       const void *data) {
    void *mappedData;
    vkMapMemory(device, bufferMemory, offset, size, 0, &mappedData);
    std::memcpy(mappedData, data, (size_t) size);
    vkUnmapMemory(device, bufferMemory);
}

void VulkanContext::createTextureImage(const void *imageData, uint32_t width, uint32_t height, uint32_t depth,
                                       VkImageType imageType, VkFormat format, VkImageTiling tiling,
                                       VkImage &outTextureImage, VkDeviceMemory &outTextureMemory,
                                       VkImageLayout textureLayout) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkDeviceSize imageSize = (VkDeviceSize) width * height * depth;

    // create staging buffer to create image in device local memory
    createBuffer(imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    updateBufferMemory(stagingBufferMemory, 0, imageSize, imageData);

    createImage(width, height, depth, imageType, format, tiling,
            // for copying and sampling in shaders
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            // TODO: updatable from cpu
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                outTextureImage, outTextureMemory);

    // layout transition from undefined
    // to transfer destination to prepare image for copying
    transitionImageLayout(outTextureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(stagingBuffer, outTextureImage, width, height, depth);

    // layout transition from transfer destination to shader readonly
    transitionImageLayout(outTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureLayout);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanContext::createImage(uint32_t width, uint32_t height, uint32_t depth,
                                VkImageType imageType, VkFormat format, VkImageTiling tiling,
                                VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                VkImage &outImage, VkDeviceMemory &outImageMemory) {

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = imageType;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = depth;
    // TODO: mipmaps
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult r = vkCreateImage(device, &imageInfo, nullptr, &outImage);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create image");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, outImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

    r = vkAllocateMemory(device, &allocInfo, nullptr, &outImageMemory);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't allocate memory for image");
    }

    r = vkBindImageMemory(device, outImage, outImageMemory, 0);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't bind image memory");
    }
}

void VulkanContext::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                                      uint32_t depth) {
    // TODO: create _beginTempCommandBuffer and _endTempCommandBuffer
    VkCommandBuffer commandBuffer; // = _beginTempCommandBuffer(device, commandPool);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // TODO: mipmaps
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, depth};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void VulkanContext::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    // TODO: create _beginTempCommandBuffer and _endTempCommandBuffer
    VkCommandBuffer commandBuffer; // = _beginTempCommandBuffer(device, commandPool);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // TODO: mipmaps
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        // undefined to transfer destination

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        // transfer destination to fragment shader

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw VulkanException("Unimplemented layout transition");
    }

    vkCmdPipelineBarrier(
            commandBuffer, sourceStage, destinationStage,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void VulkanContext::createImageView(VkImageView &outImageView, VkImage image, VkImageViewType viewType,
                                    VkFormat format, const VkImageSubresourceRange &subResourceRange,
                                    VkComponentMapping components) {
    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = viewType;
    imageViewInfo.format = format;
    imageViewInfo.components = components;
    imageViewInfo.subresourceRange = subResourceRange;

    VkResult r = vkCreateImageView(device, &imageViewInfo, nullptr, &outImageView);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create image view");
    }
}