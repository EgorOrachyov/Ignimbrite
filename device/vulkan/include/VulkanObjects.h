/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANOBJECTS_H
#define IGNIMBRITELIBRARY_VULKANOBJECTS_H

#include <ignimbrite/RenderDevice.h>
#include <ignimbrite/Optional.h>
#include <VulkanDescriptorAllocator.h>

namespace ignimbrite {

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

    struct VulkanDrawList {
        VkCommandBuffer buffer = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        ObjectID surfaceId;
        bool surfaceAttached = false;
        bool frameBufferAttached = false;
        bool pipelineAttached = false;
        bool uniformSetAttached = false;
        bool vertexBufferAttached = false;
        bool indexBufferAttached = false;
        bool drawCalled = false;
    };

    struct VulkanUniformBuffer {
        BufferUsage usage;
        uint32 size;
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    struct VulkanUniformLayout {
        VulkanDescriptorAllocator allocator;
        VulkanDescriptorProperties properties;
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
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANOBJECTS_H