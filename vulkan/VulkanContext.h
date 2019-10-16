//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_VULKANCONTEXT_H
#define VULKANRENDERER_VULKANCONTEXT_H

#include <Compilation.h>
#include <Types.h>
#include <VulkanQueueFamilyIndices.h>
#include <VulkanApplication.h>
#include <VulkanSwapChainSupportDetails.h>

/**
 * Contains all the vulkan specific functionality for
 * creating instance, device, swap-chain and etc.
 */
class VulkanContext {
public:
    /**
     * Initialize Vulkan instance for the application
     * @param app Window application instance info
     */
    VulkanContext(VulkanApplication &app);
    ~VulkanContext();

private:
    void _createInstance();
    void _destroyInstance();
    void _checkSupportedExtensions();
    bool _checkValidationLayers();
    void _fillRequiredExt(uint32 count, const char *const *ext);
    void _setupDebugMessenger();
    void _destroyDebugMessenger();
    void _pickPhysicalDevice();
    bool _isDeviceSuitable(VkPhysicalDevice device);
    bool _checkDeviceExtensionSupport(VkPhysicalDevice device);
    void _querySwapChainSupport(VkPhysicalDevice device, VulkanSwapChainSupportDetails &details);
    void _findQueueFamilies(VkPhysicalDevice device, VulkanQueueFamilyIndices& indices);
    void _createLogicalDevice();
    void _destroyLogicalDevice();
    void _setupQueue();
    void _createSurface();
    void _destroySurface();

    static void _outDeviceInfoVerbose(
            VkPhysicalDevice device);

    static VkResult _createDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
            const VkAllocationCallbacks *pAllocator,
            VkDebugUtilsMessengerEXT *pDebugMessenger);

    static void _destroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
private:

    std::vector<const char*> mRequiredExtensions;
    const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };
    const bool mEnableValidationLayers;

    VkInstance mInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;

    VulkanApplication &mApp;
    VulkanWindow &mWindow;
    VulkanQueueFamilyIndices mIndices;

};


#endif //VULKANRENDERER_VULKANCONTEXT_H
