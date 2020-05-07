/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANOBJECTS_H
#define IGNIMBRITE_VULKANOBJECTS_H

#include <IRenderDevice.h>
#include <Optional.h>
#include <VulkanDescriptorAllocator.h>
#include <vk_mem_alloc.h>

namespace ignimbrite {

    struct VulkanVertexLayout {
        std::vector<VkVertexInputBindingDescription> vkBindings;
        std::vector<VkVertexInputAttributeDescription> vkAttributes;
    };

    struct VulkanAllocation {
        VkDeviceMemory memory;
        uint32 offset;
        VmaAllocation vmaAllocation;
    };

    struct VulkanVertexBuffer {
        BufferUsage usage;
        uint32 size;
        VkBuffer vkBuffer;
        VulkanAllocation allocation;
    };

    struct VulkanIndexBuffer {
        BufferUsage usage;
        uint32 size;
        VkBuffer vkBuffer;
        VulkanAllocation allocation;
    };

    struct VulkanTextureObject {
        VkImage image;
        VulkanAllocation allocation;
        VkImageView imageView;
        VkImageType type;
        VkImageLayout layout;
        VkFormat format;
        uint32 width;
        uint32 height;
        uint32 depth;
        uint32 mipmaps;
        VkImageUsageFlags usageFlags;
        bool isCubemap;
    };

    struct VulkanUniformBuffer {
        BufferUsage usage;
        uint32 size;
        VkBuffer buffer;
        VulkanAllocation allocation;
    };

    struct VulkanUniformLayout {
        VulkanDescriptorAllocator allocator;
        VulkanDescriptorProperties properties;
    };

    struct VulkanUniformSet {
        ID<IRenderDevice::UniformLayout> uniformLayout;
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
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANOBJECTS_H