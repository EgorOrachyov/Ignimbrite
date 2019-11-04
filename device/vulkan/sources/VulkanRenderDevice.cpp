//
// Created by Egor Orachyov on 2019-11-02.
//

#include "include/VulkanRenderDevice.h"
#include "include/VulkanDefinitions.h"
#include <vulkan/vulkan.h>
#include <exception>

RenderDevice::ID VulkanRenderDevice::createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) {
    VertexLayoutBatch batch;

    auto& vertBindings = batch.vertBindings;
    auto& vertAttributes = batch.vertAttributes;

    for (size_t i = 0; i < vertexBuffersDesc.size(); i++) {
        const VertexBufferLayoutDesc& desc = vertexBuffersDesc[i];

        VkVertexInputBindingDescription bindingDesc;
        bindingDesc.binding = (uint32_t)i;
        bindingDesc.inputRate = VulkanDefinitions::vertexInputRate(desc.usage);
        bindingDesc.stride = (uint32_t)desc.stride;

        vertBindings.push_back(bindingDesc);

        for (size_t j = 0; j < desc.attributes.size(); j++)
        {
            const VertexAttributeDesc &attr = desc.attributes[j];

            VkVertexInputAttributeDescription attrDesc;
            attrDesc.binding = bindingDesc.binding;
            attrDesc.format = VulkanDefinitions::dataFormat(attr.format);
            attrDesc.location = attr.location;
            attrDesc.offset = attr.offset;

            vertAttributes.push_back(attrDesc);
        }
    }

    return vertexLayoutBatches.move(batch);
}

void VulkanRenderDevice::destroyVertexLayout(RenderDevice::ID layout) {
    vertexLayoutBatches.remove(layout);
}

RenderDevice::ID VulkanRenderDevice::createVertexBuffer(BufferUsage type, uint32 size, const void *data) {
    VulkanContext::BufferObject vertexBufferObj = {};
    context.createBufferObject(type, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, data, vertexBufferObj);

    return vertexBuffers.move(vertexBufferObj);
}

RenderDevice::ID VulkanRenderDevice::createIndexBuffer(BufferUsage type, uint32 size, const void *data) {
    VulkanContext::BufferObject indexBufferObj = {};
    context.createBufferObject(type, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, data, indexBufferObj);

    return indexBuffers.move(indexBufferObj);
}

void VulkanRenderDevice::updateVertexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
    const VkDeviceMemory &bufferMemory = vertexBuffers.get(bufferId).memory;
    context.updateBufferMemory(bufferMemory, data, size, offset);
}

void VulkanRenderDevice::updateIndexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
    const VkDeviceMemory &bufferMemory = indexBuffers.get(bufferId).memory;
    context.updateBufferMemory(bufferMemory, data, size, offset);
}

void VulkanRenderDevice::destroyVertexBuffer(RenderDevice::ID bufferId) {
    const VkDevice &device = context.getDevice();
    VulkanContext::BufferObject &vb = vertexBuffers.get(bufferId);

    vkDestroyBuffer(device, vb.buffer, nullptr);
    vkFreeMemory(device, vb.memory, nullptr);

    vertexBuffers.remove(bufferId);
}

void VulkanRenderDevice::destroyIndexBuffer(RenderDevice::ID bufferId) {
    const VkDevice &device = context.getDevice();
    VulkanContext::BufferObject &ib = indexBuffers.get(bufferId);

    vkDestroyBuffer(device, ib.buffer, nullptr);
    vkFreeMemory(device, ib.memory, nullptr);

    indexBuffers.remove(bufferId);
}

RenderDevice::ID VulkanRenderDevice::createTexture(const RenderDevice::TextureDesc &textureDesc) {
    VkFormat format = VulkanDefinitions::dataFormat(textureDesc.format);
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageType imageType = VulkanDefinitions::imageType(textureDesc.type);
    VkImageViewType viewType = VulkanDefinitions::imageViewType(textureDesc.type);

    if (textureDesc.usageFlags & (uint32)TextureUsageBit::ShaderSampling)
    {
        ImageObject imo;

        context.createTextureImage(textureDesc.data,
            textureDesc.width, textureDesc.height, textureDesc.depth,
            imageType, format, VK_IMAGE_TILING_OPTIMAL,
            imo.image, imo.imageMemory, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = aspectMask;
        // TODO: mipmaps
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        context.createImageView(imo.imageView, imo.image, viewType, format, subresourceRange);

        return imageObjects.move(imo);
    }
    else if (textureDesc.usageFlags & (uint32)TextureUsageBit::DepthStencilAttachment)
    {
        ImageObject depthStencil;

        // get properties of depth stencil format
        const VkFormatProperties &properties = context.getDeviceFormatProperties(format);

        VkImageTiling tiling;

        if (properties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            tiling = VK_IMAGE_TILING_LINEAR;
        }
        else if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            tiling = VK_IMAGE_TILING_OPTIMAL;
        }
        else
        {
            throw std::exception("Vulkan::Unsupported depth format");
        }

        context.createImage(textureDesc.width, textureDesc.height, textureDesc.depth,
            imageType, format, tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            // depth stencil buffer is device local
            // TODO: make visible from cpu
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthStencil.image, depthStencil.imageMemory);

        VkComponentMapping components;
        components.r = VK_COMPONENT_SWIZZLE_R;
        components.g = VK_COMPONENT_SWIZZLE_G;
        components.b = VK_COMPONENT_SWIZZLE_B;
        components.a = VK_COMPONENT_SWIZZLE_A;

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        context.createImageView(depthStencil.imageView, depthStencil.image,
            viewType, format, subresourceRange, components);

        return imageObjects.move(depthStencil);
    }
    else if (textureDesc.usageFlags & (uint32)TextureUsageBit::ColorAttachment)
    {
        throw std::exception("Vulkan::Color attachments must created with swapchain");
    }
    else
    {
        throw InvalidEnum();
        return RenderDevice::ID();
    }
}

void VulkanRenderDevice::destroyTexture(RenderDevice::ID textureId) {
    const VkDevice& device = context.getDevice();
    ImageObject &imo = imageObjects.get(textureId);

    vkDestroyImageView(device, imo.imageView, nullptr);
    vkDestroyImage(device, imo.image, nullptr);
    vkFreeMemory(device, imo.imageMemory, nullptr);

    imageObjects.remove(textureId);
}

RenderDevice::ID VulkanRenderDevice::createSampler(const RenderDevice::SamplerDesc &samplerDesc) {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.minFilter = VulkanDefinitions::filter(samplerDesc.min);
    samplerInfo.magFilter = VulkanDefinitions::filter(samplerDesc.mag);
    samplerInfo.addressModeU = VulkanDefinitions::samplerAddressMode(samplerDesc.u);
    samplerInfo.addressModeV = VulkanDefinitions::samplerAddressMode(samplerDesc.v);
    samplerInfo.addressModeW = VulkanDefinitions::samplerAddressMode(samplerDesc.w);
    samplerInfo.anisotropyEnable = samplerDesc.useAnisotropy ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = samplerDesc.anisotropyMax;
    samplerInfo.borderColor = VulkanDefinitions::borderColor(samplerDesc.color);
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSampler sampler;
    VkResult r = vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &sampler);
    if (r != VK_SUCCESS)
    {
        throw InvalidEnum();
    }

    return samplers.add(sampler);
}

void VulkanRenderDevice::destroySampler(RenderDevice::ID samplerId) {
    vkDestroySampler(context.getDevice(), samplers.get(samplerId), nullptr);
    samplers.remove(samplerId);
}
