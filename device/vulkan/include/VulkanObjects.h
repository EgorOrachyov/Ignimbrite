//
// Created by Egor Orachyov on 2019-11-11.
//

#ifndef IGNIMBRITELIBRARY_VULKANOBJECTS_H
#define IGNIMBRITELIBRARY_VULKANOBJECTS_H

#include <vulkan/vulkan.h>
#include <ignimbrite/RenderDevice.h>
#include <ignimbrite/Optional.h>

namespace ignimbrite {

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
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        ObjectID framebufferFormatId;
        uint32 width = 0;
        uint32 height = 0;
    };

    struct VulkanDrawList {
        VkCommandBuffer buffer = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        ObjectID surfaceId;
        bool frameBufferAttached = false;
        bool pipelineAttached = false;
        bool uniformSetAttached = false;
        bool vertexBufferAttached = false;
        bool indexBufferAttached = false;
        bool drawCalled = false;
    };

    /** Associated with chain data also needed for screen rendering (managed automatically) */
    struct VulkanSwapChain {
        VkSwapchainKHR swapChainKHR;
        VkExtent2D extent;
        VulkanFrameBufferFormat framebufferFormat;
        std::vector<VkFramebuffer> framebuffers;
        /** Images and views color attachment 0 */
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        /** Images and views for depth buffer (created by hand) */
        std::vector<VkImage> depthStencilImages;
        std::vector<VkImageView> depthStencilImageViews;
        std::vector<VkDeviceMemory> depthStencilImageMemory;
    };

    /** Represents window drawing area, created by native OS window system */
    struct VulkanSurface {
        std::string name;
        uint32 width;
        uint32 height;
        uint32 widthFramebuffer;
        uint32 heightFramebuffer;
        uint32 presentsFamily;
        VkQueue presentQueue;
        VkQueue graphicsQueue;
        /** Surface created vie extension for specific WSI */
        VkSurfaceKHR surface;
        VkPresentModeKHR presentMode;
        VkSurfaceFormatKHR surfaceFormat;
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        VulkanSwapChain swapChain;
        /** Swap buffer data */
        uint32 currentImageIndex = 0;
        uint32 currentFrameIndex = 0;
        uint32 maxFramesInFlight = 3;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
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
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    struct VulkanShader {
        VkShaderModule module;
        VkShaderStageFlagBits shaderStage;
    };

    struct VulkanShaderProgram {
        std::vector<VulkanShader> shaders;
    };

    struct VulkanGraphicsPipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        bool withSurfaceOnly = false;
        RenderDevice::ID surface;
        RenderDevice::ID program;
        RenderDevice::ID uniformLayout;
        RenderDevice::ID vertexLayout;
        RenderDevice::ID framebufferFormat;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANOBJECTS_H