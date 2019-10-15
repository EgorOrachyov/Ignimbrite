//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_VULKANCONTEXT_H
#define VULKANRENDERER_VULKANCONTEXT_H

#include <Compilation.h>
#include <Types.h>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <VulkanQueueFamilyIndices.h>

/**
 * Contains all the vulkan specific functionality for
 * creating instance, device, swap-chain and etc.
 */
class VulkanContext {
public:
    /**
     * Initialize Vulkan instance for the application
     *
     * Extensions info could be got from Window API, such as GLFW.
     * This info needed for communication between renderer and window canvas.
     *
     * @param appName Name of the application
     * @param extensionsCount Number of extensions to be used
     * @param extensions Actual extensions list
     */
    VulkanContext(std::string appName, bool enableValidation = true, uint32 extensionsCount = 0, const char *const *extensions = nullptr);
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
    static bool _isDeviceSuitable(VkPhysicalDevice device);
    static void _findQueueFamilies(VkPhysicalDevice device, VulkanQueueFamilyIndices& indices);
    static void _outDeviceInfoVerbose(VkPhysicalDevice device);
    void _createLogicalDevice();
    void _destroyLogicalDevice();
    void _setupQueue();

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

    /** Required extension by window system */
    std::vector<const char*> mRequiredExtensions;
    /** Required validation layers for debug mode */
    const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };
    const bool mEnableValidationLayers;

    VkInstance mInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkQueue mGraphicsQueue;

    std::string mApplicationName;
    VulkanQueueFamilyIndices mIndices;

};


#endif //VULKANRENDERER_VULKANCONTEXT_H
