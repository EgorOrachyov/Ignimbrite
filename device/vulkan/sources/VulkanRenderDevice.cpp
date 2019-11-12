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
    VkImageType imageType = VulkanDefinitions::imageType(textureDesc.type);
    VkImageViewType viewType = VulkanDefinitions::imageViewType(textureDesc.type);
    VkImageUsageFlags usageFlags = VulkanDefinitions::imageUsageFlags(textureDesc.usageFlags);

    VulkanTextureObject texture = {};
    texture.type = imageType;
    texture.format = format;
    texture.usageFlags = usageFlags;
    texture.width = textureDesc.width;
    texture.height = textureDesc.height;
    texture.depth = textureDesc.depth;
    texture.mipmaps = textureDesc.mipmaps;

    if (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT)
    {
        if (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT &&
            usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT == 0)
        {
            // if texture can be sampled in shader and it's a color attachment

            // TODO
        }
        else if (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT == 0 &&
            usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            // if texture can be sampled in shader and it's a depth stencil attachment

            // TODO
        }
        else if (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT == 0 &&
            usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT == 0)
        {
            // if texture can be sampled in shader and it's not an attachment

            // create texture image with mipmaps and allocate memory
            context.createTextureImage(textureDesc.data,
                textureDesc.width, textureDesc.height, textureDesc.depth, textureDesc.mipmaps,
                imageType, format, VK_IMAGE_TILING_OPTIMAL,
                texture.image, texture.imageMemory, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            // create image view
            VkImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = textureDesc.mipmaps;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            context.createImageView(texture.imageView, texture.image, viewType, format, subresourceRange);

            return mTextureObjects.move(texture);
        }
        else
        {
            VulkanException("VulkanRenderDevice::Texture can't be color and depth stencil attachment at the same time");
        }
    }
    else if (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT &&
        usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT == 0)
    {
        // if depth stencil attahment

        context.createDepthStencilBuffer(textureDesc.width, textureDesc.height, textureDesc.depth,
            imageType, format, viewType, texture.image, texture.imageMemory, texture.imageView);

        return mTextureObjects.move(texture);
    }
    else if (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT &&
        usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT == 0)
    {
        // if color attachment

        // TODO
        texture.image = VK_NULL_HANDLE;
        texture.imageMemory = VK_NULL_HANDLE;
        texture.imageView = VK_NULL_HANDLE;

        return mTextureObjects.move(texture);
    }
    else
    {
        throw VulkanException("Texture can't be color and depth stencil attachment at the same time");
    }
}

void VulkanRenderDevice::destroyTexture(RenderDevice::ID textureId) {
    const VkDevice &device = context.device;
    VulkanTextureObject &imo = mTextureObjects.get(textureId);

    vkDestroyImageView(device, imo.imageView, nullptr);
    vkDestroyImage(device, imo.image, nullptr);
    vkFreeMemory(device, imo.imageMemory, nullptr);

    mTextureObjects.remove(textureId);
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

RenderDevice::ID VulkanRenderDevice::createFramebuffer(const std::vector<RenderDevice::ID>& attachmentIds, 
                                                            RenderDevice::ID framebufferFormatId) {
    // get framebuffer attachemnts info
    std::vector<VkImageView> framebufferAttchViews;

    VulkanTextureObject &firstAtt = mTextureObjects.get(attachmentIds[0]);
    uint32 width = firstAtt.width;
    uint32 height = firstAtt.height;
    framebufferAttchViews.push_back(firstAtt.imageView);

    for (size_t i = 1; i < attachmentIds.size(); i++)
    {
        VulkanTextureObject &att = mTextureObjects.get(attachmentIds[i]);
        if (width != att.width || height != att.height)
        {
            throw VulkanException("Framebuffer attachments must be same size");
        }

        framebufferAttchViews.push_back(att.imageView);
    }

    // TODO: get framebuffer attachement descriptions using "framebufferFormatId"
    const std::vector<FramebufferAttachmentDesc> attchDescs = {};

    // parse 
    std::vector<VkAttachmentDescription> vAttchDescs;
    std::vector<VkAttachmentReference> vColorAttchRefs;
    VkAttachmentReference vDepthStencilAttchRef;
    bool isDepthStencilSet = false;

    for (uint32_t i = 0; i < attchDescs.size(); i++)
    {
        const FramebufferAttachmentDesc& att = attchDescs[i];

        VkAttachmentDescription desc;
        desc.format = VulkanDefinitions::dataFormat(att.format);
        desc.samples = VulkanDefinitions::sampleCount(att.samples);
        desc.flags = 0;

        VkAttachmentReference ref;
        ref.attachment = i;

        if (att.type == AttachmentType::Color)
        {
            // TODO: should be defined by user
            
            // clear at the start of render pass
            desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            // leave rendering result in this buffer
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            // initial layout is undefined
            desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            // final layout must be appropriate for present operation
            desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            vColorAttchRefs.push_back(ref);
        }
        else
        {
            if (isDepthStencilSet)
            {
                throw VulkanException("Vulkan::Only one depth stencil attachment can be set to subpass");
            }

            // TODO: should be defined by user

            // clear at the start of render pass
            desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            // leave same as it will not be presented
            desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            vDepthStencilAttchRef = ref;
            isDepthStencilSet = true;
        }

        vAttchDescs.push_back(desc);
    }

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = NULL;
    framebufferInfo.flags = 0;
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;
    framebufferInfo.attachmentCount = framebufferAttchViews.size();
    framebufferInfo.pAttachments = framebufferAttchViews.data();
    framebufferInfo.renderPass = context.createRenderPass(vAttchDescs, vColorAttchRefs,
        vDepthStencilAttchRef, isDepthStencilSet);

    VkFramebuffer framebuffer;
    VkResult r = vkCreateFramebuffer(context.device, &framebufferInfo, nullptr, &framebuffer);
    
    if (r != VK_SUCCESS)
    {
        throw VulkanException("Can't create framebuffer");
    }

    return mFramebuffers.add(framebuffer);
}

void VulkanRenderDevice::destroyFramebuffer(RenderDevice::ID framebufferId) {
    VkFramebuffer framebuffer = mFramebuffers.get(framebufferId);
    vkDestroyFramebuffer(context.device, framebuffer, nullptr);

    mFramebuffers.remove(framebufferId);
}