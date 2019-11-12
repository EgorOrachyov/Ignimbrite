//
// Created by Egor Orachyov on 2019-11-02.
//

#include <VulkanRenderDevice.h>
#include <VulkanDefinitions.h>
#include <vulkan/vulkan.h>
#include <exception>

VulkanRenderDevice::VulkanRenderDevice(uint32 extensionsCount, const char *const *extensions) {

}

RenderDevice::ID VulkanRenderDevice::createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) {
    VulkanVertexLayout layout;

    auto &vertBindings = layout.vkBindings;
    auto &vertAttributes = layout.vkAttributes;

    for (uint32 i = 0; i < vertexBuffersDesc.size(); i++) {
        const VertexBufferLayoutDesc &desc = vertexBuffersDesc[i];

        VkVertexInputBindingDescription bindingDesc;
        bindingDesc.binding = i;
        bindingDesc.inputRate = VulkanDefinitions::vertexInputRate(desc.usage);
        bindingDesc.stride = desc.stride;

        vertBindings.push_back(bindingDesc);

        for (uint32 j = 0; j < desc.attributes.size(); j++) {
            const VertexAttributeDesc &attr = desc.attributes[j];

            VkVertexInputAttributeDescription attrDesc;
            attrDesc.binding = bindingDesc.binding;
            attrDesc.format = VulkanDefinitions::dataFormat(attr.format);
            attrDesc.location = attr.location;
            attrDesc.offset = attr.offset;

            vertAttributes.push_back(attrDesc);
        }
    }

    return mVertexLayouts.move(layout);
}

void VulkanRenderDevice::destroyVertexLayout(RenderDevice::ID layout) {
    mVertexLayouts.remove(layout);
}

RenderDevice::ID VulkanRenderDevice::createVertexBuffer(BufferUsage type, uint32 size, const void *data) {
    VulkanVertexBuffer vertexBuffer = { };
    vertexBuffer.size = size;
    vertexBuffer.usage = type;

    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    if (type == BufferUsage::Dynamic) {
        context.createBuffer(size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vertexBuffer.vkBuffer, vertexBuffer.vkDeviceMemory
        );
        context.updateBufferMemory(vertexBuffer.vkDeviceMemory, 0, size, data);
    } else {
        context.createBufferLocal(data, size, usage, vertexBuffer.vkBuffer, vertexBuffer.vkDeviceMemory);
    }

    return mVertexBuffers.move(vertexBuffer);
}

RenderDevice::ID VulkanRenderDevice::createIndexBuffer(BufferUsage type, uint32 size, const void *data) {
    VulkanIndexBuffer indexBuffer = { };
    indexBuffer.size = size;
    indexBuffer.usage = type;

    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    if (type == BufferUsage::Dynamic) {
        context.createBuffer(size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     indexBuffer.vkBuffer, indexBuffer.vkDeviceMemory
        );
        context.updateBufferMemory(indexBuffer.vkDeviceMemory, 0, size, data);
    } else {
        context.createBufferLocal(data, size, usage, indexBuffer.vkBuffer, indexBuffer.vkDeviceMemory);
    }

    return mIndexBuffers.move(indexBuffer);
}

void VulkanRenderDevice::updateVertexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
    const VulkanVertexBuffer& buffer = mVertexBuffers.get(bufferId);
    const VkDeviceMemory &memory = buffer.vkDeviceMemory;

    if (buffer.usage != BufferUsage::Dynamic) {
        throw VulkanException("Attempt to update static vertex buffer");
    }

    if (size + offset > buffer.size) {
        throw VulkanException("Attempt to update out-of-buffer memory region for vertex buffer");
    }

    context.updateBufferMemory(memory, offset, size, data);
}

void VulkanRenderDevice::updateIndexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
    const VulkanIndexBuffer& buffer = mIndexBuffers.get(bufferId);
    const VkDeviceMemory &memory = buffer.vkDeviceMemory;

    if (buffer.usage != BufferUsage::Dynamic) {
        throw VulkanException("Attempt to update static index buffer");
    }

    if (size + offset > buffer.size) {
        throw VulkanException("Attempt to update out-of-buffer memory region for index buffer");
    }

    context.updateBufferMemory(memory, offset, size, data);
}

void VulkanRenderDevice::destroyVertexBuffer(RenderDevice::ID bufferId) {
    const VkDevice &device = context.device;
    VulkanVertexBuffer& buffer = mVertexBuffers.get(bufferId);

    vkDestroyBuffer(device, buffer.vkBuffer, nullptr);
    vkFreeMemory(device, buffer.vkDeviceMemory, nullptr);

    mVertexBuffers.remove(bufferId);
}

void VulkanRenderDevice::destroyIndexBuffer(RenderDevice::ID bufferId) {
    const VkDevice &device = context.device;
    VulkanIndexBuffer& buffer = mIndexBuffers.get(bufferId);

    vkDestroyBuffer(device, buffer.vkBuffer, nullptr);
    vkFreeMemory(device, buffer.vkDeviceMemory, nullptr);

    mVertexBuffers.remove(bufferId);
}

RenderDevice::ID VulkanRenderDevice::createTexture(const RenderDevice::TextureDesc &textureDesc) {
    VkFormat format = VulkanDefinitions::dataFormat(textureDesc.format);
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageType imageType = VulkanDefinitions::imageType(textureDesc.type);
    VkImageViewType viewType = VulkanDefinitions::imageViewType(textureDesc.type);

    if (textureDesc.usageFlags & (uint32) TextureUsageBit::ShaderSampling) {
        VulkanImageObject imo;

        context.createTextureImage(textureDesc.data,
                                   textureDesc.width, textureDesc.height, textureDesc.depth, textureDesc.mipmaps,
                                   imageType, format, VK_IMAGE_TILING_OPTIMAL,
                                   imo.image, imo.imageMemory, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = textureDesc.mipmaps;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        context.createImageView(imo.imageView, imo.image, viewType, format, subresourceRange);

        return mImageObjects.move(imo);
    } else if (textureDesc.usageFlags & (uint32) TextureUsageBit::DepthStencilAttachment) {
        VulkanImageObject depthStencil;

        // get properties of depth stencil format
        const VkFormatProperties &properties = context.getDeviceFormatProperties(format);

        VkImageTiling tiling;

        if (properties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_LINEAR;
        } else if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
        } else {
            throw VulkanException("Unsupported depth format");
        }

        context.createImage(textureDesc.width, textureDesc.height, textureDesc.depth, 1,
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

        return mImageObjects.move(depthStencil);
    } else if (textureDesc.usageFlags & (uint32) TextureUsageBit::ColorAttachment) {
        throw VulkanException("Color attachments must created with swapchain");
    } else {
        throw VulkanException("Failed to create texture object");
    }
}

void VulkanRenderDevice::destroyTexture(RenderDevice::ID textureId) {
    const VkDevice &device = context.device;
    VulkanImageObject &imo = mImageObjects.get(textureId);

    vkDestroyImageView(device, imo.imageView, nullptr);
    vkDestroyImage(device, imo.image, nullptr);
    vkFreeMemory(device, imo.imageMemory, nullptr);

    mImageObjects.remove(textureId);
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
    samplerInfo.mipmapMode = VulkanDefinitions::samplerMipmapMode(samplerDesc.mipmapMode);
    samplerInfo.mipLodBias = samplerDesc.mipLodBias;
    samplerInfo.minLod = samplerDesc.minLod;
    samplerInfo.maxLod = samplerDesc.maxLod;

    VkSampler sampler;
    VkResult r = vkCreateSampler(context.device, &samplerInfo, nullptr, &sampler);

    if (r != VK_SUCCESS) {
        throw VulkanException("Failed to create sampler object");
    }

    return mSamplers.add(sampler);
}

void VulkanRenderDevice::destroySampler(RenderDevice::ID samplerId) {
    vkDestroySampler(context.device, mSamplers.get(samplerId), nullptr);
    mSamplers.remove(samplerId);
}

RenderDevice::ID VulkanRenderDevice::getSurface(const std::string &surfaceName) {
    for (auto i = mSurfaces.begin(); i != mSurfaces.end(); ++i) {
        auto& window = *i;
        if (window.name == surfaceName) {
            return i.getID();
        }
    }
    return INVALID;
}

void VulkanRenderDevice::getSurfaceSize(RenderDevice::ID surface, uint32 &width, uint32 &height) {
    auto& window = mSurfaces.get(surface);

    width = window.width;
    height = window.height;
}
