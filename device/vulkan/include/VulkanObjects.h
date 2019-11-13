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
    Optional<uint32> presentsFamily;

    bool isCompleteGT() {
        return graphicsFamily.hasValue() &&
               transferFamily.hasValue();
    }

    bool isCompleteGTP() {
        return graphicsFamily.hasValue() &&
               transferFamily.hasValue() &&
                presentsFamily.hasValue();
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
    VkFormat format;
    uint32 width;
    uint32 height;
    uint32 depth;
    uint32 mipmaps;
    VkImageUsageFlags usageFlags;
};

struct VulkanSurface {
    uint32 width;
    uint32 height;
    uint32 widthFramebuffer;
    uint32 heightFramebuffer;
    std::string name;
    bool tripleBuffering = false;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkFormat surfaceFormat;
    VkColorSpaceKHR colorSpace;
    VkPresentModeKHR presentMode;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView > swapChainImageViews;
};

struct VulkanFrameBufferFormat {
    VkRenderPass renderPass;
};

#endif //RENDERINGLIBRARY_VULKANOBJECTS_H
