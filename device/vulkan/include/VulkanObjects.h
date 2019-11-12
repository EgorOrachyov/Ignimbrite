//
// Created by Egor Orachyov on 2019-11-11.
//

#ifndef RENDERINGLIBRARY_VULKANOBJECTS_H
#define RENDERINGLIBRARY_VULKANOBJECTS_H

#include <vulkan/vulkan.h>
#include <renderer/RenderDevice.h>

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
    VkSurfaceKHR surface;
};

#endif //RENDERINGLIBRARY_VULKANOBJECTS_H
