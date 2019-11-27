//
// Created by Egor Orachyov on 2019-11-11.
//

#ifndef RENDERINGLIBRARY_VULKANOBJECTS_H
#define RENDERINGLIBRARY_VULKANOBJECTS_H

#include <vulkan/vulkan.h>
#include <renderer/RenderDevice.h>
#include <renderer/Optional.h>

struct VulkanQueueFamilyIndices {
    Optional<uint32> graphicsFamily;
    Optional<uint32> transferFamily;

    bool isComplete() {
        return graphicsFamily.hasValue() &&
               transferFamily.hasValue();
    }
};

struct VulkanVertexLayout {
    std::vector<VkVertexInputBindingDescription> vkBindings;
    std::vector<VkVertexInputAttributeDescription> vkAttributes;
};

struct VulkanVertexBuffer {
    BufferUsage usage;
    uint32 size;
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
};

struct VulkanIndexBuffer {
    BufferUsage usage;
    uint32 size;
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
};

struct VulkanTextureObject {
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
    VkImageType type;
    VkImageLayout layout;
    VkFormat format;
    uint32 width;
    uint32 height;
    uint32 depth;
    uint32 mipmaps;
    VkImageUsageFlags usageFlags;
};

struct VulkanFrameBufferFormat {
    VkRenderPass renderPass;
    uint32 numOfAttachments;
    bool useDepthStencil;
};

struct VulkanFrameBuffer {
    VkFramebuffer framebuffer;
    VkRenderPass renderPass;
    uint32 width;
    uint32 height;
};

struct VulkanDrawList {
    VkCommandBuffer buffer = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    bool frameBufferAttached = false;
    bool pipelineAttached = false;
    bool uniformSetAttached = false;
    bool vertexBufferAttached = false;
    bool indexBufferAttached = false;
    bool drawCalled = false;
};

struct VulkanSurface {
    std::string name;
    uint32 width;
    uint32 height;
    uint32 widthFramebuffer;
    uint32 heightFramebuffer;
    bool tripleBuffering = false;
    uint32 presentsFamily;
    VkQueue presentQueue;
    VkQueue graphicsQueue;
    /// Surface created vie extension for specific WSI
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormat;
    /// Associated with chain data also needed for screen rendering (managed automatically)
    VkSwapchainKHR swapChain;
    VkExtent2D swapChainExtent;
    VulkanFrameBufferFormat framebufferFormat;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> drawCmdBuffers;
    uint32 currentImageIndex = 0;
    // For swapchain image presentation
    std::vector<VkSemaphore> presentSemaphores;
    // For submitting and executing cmd buffer
    std::vector<VkSemaphore> renderSemaphores;
    std::vector<VkFence> waitFences;
};

struct VulkanUniformBuffer {
    BufferUsage usage;
    uint32 size;
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct VulkanDescriptorPool {
    VkDescriptorPool pool;
    uint32 allocatedSets;
    uint32 maxSets;
};

struct VulkanUniformLayout {
    VkDescriptorSetLayout setLayout;
    uint32 texturesCount;
    uint32 buffersCount;
    uint32 usedDescriptorSets;
    std::vector<VulkanDescriptorPool> pools;
    std::vector<VkDescriptorSet> freeSets;
};

struct VulkanUniformSet {
    RenderDevice::ID uniformLayout;
    VkDescriptorSet descriptorSet;
};

struct VulkanShader {
    VkShaderModule module;
    VkShaderStageFlagBits shaderStage;
};

struct VulkanShaderProgram {
    std::vector<VulkanShader> shaders;
};

struct VulkanGraphicsPipeline {
    RenderDevice::ID surface;
    bool withSurfaceOnly;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    RenderDevice::ID program;
    RenderDevice::ID uniformLayout;
    RenderDevice::ID vertexLayout;
    RenderDevice::ID framebufferFormat;
};

#endif //RENDERINGLIBRARY_VULKANOBJECTS_H
