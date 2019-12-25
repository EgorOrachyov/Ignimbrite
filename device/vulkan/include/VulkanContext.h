/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANCONTEXT_H
#define IGNIMBRITELIBRARY_VULKANCONTEXT_H

#include <ignimbrite/Optional.h>
#include <VulkanDefinitions.h>
#include <VulkanSurface.h>
#include <vector>

namespace ignimbrite {

    /** Collects info about queue families for VK device */
    struct VulkanQueueFamilyIndices {
        Optional<uint32> graphicsFamily;
        Optional<uint32> transferFamily;

        bool isComplete() {
            return graphicsFamily.hasValue() &&
                   transferFamily.hasValue();
        }
    };

    /**
     * Handles vulkan instance setup. Defines physical
     * device and creates logical device for application.
     * defines queue families, finds graphics, present and transfer queues
     */
    class VulkanContext {
    public:

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
        bool checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice);

        void findQueueFamilies(VkPhysicalDevice inPhysicalDevice, VulkanQueueFamilyIndices &indices);
        void findPresentsFamily(VulkanSurface &surface);

        VkResult createDebugUtilsMessengerEXT(
                const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                const VkAllocationCallbacks *pAllocator,
                VkDebugUtilsMessengerEXT *pDebugMessenger
        );

        void destroyDebugUtilsMessengerEXT(
                VkDebugUtilsMessengerEXT inDebugMessenger,
                const VkAllocationCallbacks *pAllocator
        );

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData
        );

        void createSwapChain(VulkanSurface &surface);
        void destroySwapChain(VulkanSurface &surface);

        void createFramebufferFormat(VulkanSurface &surface);
        void destroyFramebufferFormat(VulkanSurface &surface);

        void createFramebuffers(VulkanSurface &surface);
        void destroyFramebuffers(VulkanSurface &surface);

        void updateSurfaceCapabilities(VulkanSurface &surface);
        void resizeSurface(VulkanSurface &surface);

        void createCommandPools();
        void destroyCommandPools();

        void deviceWaitIdle();

    private:

        VulkanContext() = default;
        ~VulkanContext() = default;
        VulkanContext(VulkanContext&& context) = default;
        VulkanContext(const VulkanContext& context) = default;

    public:

        /**
         * Access vulkan context for the application.
         * Allowed only single context instance.
         * @return Vulkan context instance
         */
        static VulkanContext& getSingleton();

    public:

        /** Default number of frames in flight (how much images could be rendered simultaneously) */
        static const uint32 FRAMES_IN_FLIGHT = 2;

        /** Min image count for double buffering */
        static const uint32 SWAPCHAIN_MIN_IMAGE_COUNT = 2;

        static const VkFormat PREFERRED_FORMAT = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
        static const VkColorSpaceKHR PREFERRED_COLOR_SPACE = VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        static const VkPresentModeKHR PREFERRED_PRESENT_MODE = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;

    public:

        std::vector<const char *> requiredExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};
        const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const bool enableValidationLayers = true;

        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        VulkanQueueFamilyIndices familyIndices = {};

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties deviceProperties = {};
        VkPhysicalDeviceFeatures deviceFeatures = {};
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};

        VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
        VkCommandPool transferCommandPool = VK_NULL_HANDLE;
        VkCommandPool graphicsTempCommandPool = VK_NULL_HANDLE;
        VkCommandPool transferTempCommandPool = VK_NULL_HANDLE;

        // NOTE: do not use this vector with multithreading ??
        std::vector<VkClearValue> tempClearValues = {};

    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANCONTEXT_H
