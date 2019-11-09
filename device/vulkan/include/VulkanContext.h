//
// Created by Egor Orachyov on 2019-11-03.
//

#ifndef RENDERINGLIBRARY_VULKANCONTEXT_H
#define RENDERINGLIBRARY_VULKANCONTEXT_H

#include "VulkanDefinitions.h"
#include "VulkanStructures.h"

#include <vector>

/**
 * Handles vulkan instance setup. Defines physical
 * device and creates logical device for application.
 * defines queue families, finds graphics, present and transfer queues
 */
class VulkanContext {
public:

    VulkanContext(uint32 extensionsCount, const char *const *extensions);
    ~VulkanContext();

private:

    /* Private section: setup vulkan instance */
    void _createInstance();
    void _destroyInstance();

    void _checkSupportedExtensions();
    void _fillRequiredExt(uint32 count, const char *const *ext);

    bool _checkValidationLayers();

    void _setupDebugMessenger();
    void _destroyDebugMessenger();

    VkResult _createDebugUtilsMessengerEXT(
            const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
            const VkAllocationCallbacks *pAllocator,
            VkDebugUtilsMessengerEXT *pDebugMessenger
    );

    void _destroyDebugUtilsMessengerEXT(
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks *pAllocator
    );

    static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
    );

    void _pickPhysicalDevice();
    bool _isDeviceSuitable(VkPhysicalDevice device);
    bool _checkDeviceExtensionSupport(VkPhysicalDevice device);
    void _querySwapChainSupport(VkPhysicalDevice device, SwapChainSupportDetails &details);
    void _findQueueFamilies(VkPhysicalDevice device, QueueFamilyIndices &indices);

    void _createLogicalDevice();
    void _destroyLogicalDevice();

    void _setupQueue();

public:

    const VkPhysicalDeviceMemoryProperties &getDeviceMemoryProperties() const {}

    VkFormatProperties getDeviceFormatProperties(VkFormat format) const;

    uint32_t getMemoryTypeIndex(uint32_t memoryTypeBits, VkFlags requirementsMask) const;

    void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
            VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory
    );

    void createBufferLocal(
            const void *data,
            VkDeviceSize size, VkBufferUsageFlags usage,
            VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory
    );

    void copyBuffer(
            VkCommandPool commandPool,
            VkQueue queue,
            VkBuffer srcBuffer, VkBuffer dstBuffer,
            VkDeviceSize size
    );

    void updateBufferMemory(
            VkDeviceMemory bufferMemory,
            VkDeviceSize offset, VkDeviceSize size,
            const void *data
    );

    void createTextureImage(
            const void *imageData,
            uint32_t width, uint32_t height, uint32_t depth,
            VkImageType imageType, VkFormat format, VkImageTiling tiling,
            VkImage &outTextureImage, VkDeviceMemory &outTextureMemory,
            VkImageLayout textureLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    void createImage(
            uint32_t width, uint32_t height, uint32_t depth,
            VkImageType imageType, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage &outImage, VkDeviceMemory &outImageMemory
    );

    void copyBufferToImage(
            VkBuffer buffer,
            VkImage image,
            uint32_t width, uint32_t height, uint32_t depth
    );

    void transitionImageLayout(
            VkImage image,
            VkImageLayout oldLayout,
            VkImageLayout newLayout
    );

    void createImageView(
            VkImageView &outImageView,
            VkImage image,
            VkImageViewType viewType, VkFormat format,
            const VkImageSubresourceRange &subResourceRange,
            VkComponentMapping components = {}
    );

    /* Get functions section */
    VkInstance getInstance() const { return mInstance; }
    VkDevice getDevice() const { return mDevice; }
    VkCommandPool getCommandPool() const { return mCommandPool; }
    VkQueue getTransferQueue() const { return mTransferQueue; }

private:

    std::vector<const char *> mRequiredExtensions;
    const std::vector<const char *> mDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const std::vector<const char *> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const bool mEnableValidationLayers = true;

    VkInstance mInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    QueueFamilyIndices familyIndices;

    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;
    VkQueue mTransferQueue = VK_NULL_HANDLE;

    // TODO: init deviceMemoryProperties
    VkPhysicalDeviceMemoryProperties mDeviceMemoryProperties = {};
    // TODO: init command pool
    VkCommandPool mCommandPool = VK_NULL_HANDLE;
};


#endif //RENDERINGLIBRARY_VULKANCONTEXT_H
