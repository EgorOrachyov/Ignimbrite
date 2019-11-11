//
// Created by Egor Orachyov on 2019-11-03.
//

#ifndef RENDERINGLIBRARY_VULKANCONTEXT_H
#define RENDERINGLIBRARY_VULKANCONTEXT_H

#include <VulkanDefinitions.h>
#include <VulkanObjects.h>
#include <vector>

/**
 * Handles vulkan instance setup. Defines physical
 * device and creates logical device for application.
 * defines queue families, finds graphics, present and transfer queues
 */
struct VulkanContext {

    /* Private section: setup vulkan instance */
    void createInstance();
    void destroyInstance();
    void fillRequiredExt(uint32 count, const char *const *ext);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void destroyLogicalDevice();

    void checkSupportedExtensions();
    bool checkValidationLayers();

    void setupDebugMessenger();
    void destroyDebugMessenger();
    void outDeviceInfoVerbose();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void findQueueFamilies(VkPhysicalDevice device, VulkanQueueFamilyIndices &indices);

    VkResult createDebugUtilsMessengerEXT(
            const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
            const VkAllocationCallbacks *pAllocator,
            VkDebugUtilsMessengerEXT *pDebugMessenger
    );

    void destroyDebugUtilsMessengerEXT(
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks *pAllocator
    );

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
    );

    void createSwapChain(VulkanSurface& surface);
    void destroySwapChain();

    std::vector<const char *> requiredExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const bool enableValidationLayers = true;

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VulkanQueueFamilyIndices familyIndices = { };

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties deviceProperties = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};
    // TODO: init command pool
    VkCommandPool commandPool = VK_NULL_HANDLE;
};


#endif //RENDERINGLIBRARY_VULKANCONTEXT_H
