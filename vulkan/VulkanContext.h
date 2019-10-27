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
#include <VulkanVertex.h>

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

    void drawFrame();

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
    VkSurfaceFormatKHR _chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR _chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D _chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void _createSwapChain();
    void _destroySwapChain();
    void _createImageViews(VulkanWindow& window);
    void _destroyImageViews(VulkanWindow& window);
    void _createGraphicsPipeline();
    void _destroyGraphicsPipeline();
    VkShaderModule _createShaderModule(const std::vector<char> &code);
    void _createPipelineLayout();
    void _destroyPipelineLayout();
    void _createRenderPass();
    void _destroyRenderPass();
    void _createFramebuffers(VulkanWindow& window);
    void _destroyFramebuffers(VulkanWindow& window);
    void _createCommandPool();
    void _destroyCommandPool();
    void _createCommandBuffers(VulkanWindow &window);
    void _freeCommandBuffers(VulkanWindow& window);
    void _createSyncObjects();
    void _destroySyncObjects();
    void _waitForDevice();
    void _cleanupSwapChain();
    void _recreateSwapChain();
    void _createVertexBuffer();
    void _destroyVertexBuffer();
    void _createIndexBuffer();
    void _destroyIndexBuffer();
    void _createUniformBuffers();
    void _destroyUniformBuffers();
    void _updateUniformBuffer(uint32 currentImage);
    void _createDescriptorPool();
    void _destroyDescriptorPool();
    void _createDescriptorSets();
    uint32 _findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);
    void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void _copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void _createImage(uint32 width, uint32 height, VkFormat format,
                      VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView _createImageView(VkImage image, VkFormat format);
    void _createDescriptorSetLayout();
    void _destroyDescriptorSetLayout();
    void _createTextureImage();
    void _destroyTextureImage();
    void _createTextureImageView();
    void _destroyTextureImageView();
    void _transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void _copyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height);
    VkCommandBuffer _beginSingleTimeCommands();
    void _endSingleTimeCommands(VkCommandBuffer commandBuffer);

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
    const int32 MAX_FRAMES_IN_FLIGHT = 2;

    VkInstance mInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;

    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;

    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mFlightFences;
    uint32 mCurrentFrame = 0;
    uint64 mFramesCount = 0;

    const std::vector<VulkanVertex> mVertices = {
            { {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f} },
            { {0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f} },
            { {0.5f,   0.5f}, {0.0f, 0.0f, 1.0f} },
            { {-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f} }
    };

    const std::vector<uint16_t> mIndices = {
            0, 1, 2, 2, 3, 0
    };

    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    VkBuffer mVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer mIndexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;
    std::vector<VkBuffer> mUniformBuffers;
    std::vector<VkDeviceMemory> mUniformBuffersMemory;
    VkImage mTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory mTextureImageMemory = VK_NULL_HANDLE;
    VkImageView mTextureImageView = VK_NULL_HANDLE;

    VulkanApplication &mApp;
    VulkanWindow &mWindow;
    VulkanQueueFamilyIndices mQueueIndices;

};


#endif //VULKANRENDERER_VULKANCONTEXT_H
