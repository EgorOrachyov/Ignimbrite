//
// Created by Egor Orachyov on 2019-11-02.
//

#include <VulkanRenderDevice.h>
#include <VulkanDefinitions.h>
#include <VulkanUtils.h>
#include <vulkan/vulkan.h>
#include <exception>

VulkanRenderDevice::VulkanRenderDevice(uint32 extensionsCount, const char *const *extensions) {
    context.fillRequiredExt(extensionsCount, extensions);
    context.createInstance();
    context.setupDebugMessenger();
    context.pickPhysicalDevice();
    context.createLogicalDevice();
}

VulkanRenderDevice::~VulkanRenderDevice() {
    context.destroyLogicalDevice();
    context.destroyDebugMessenger();
    context.destroyInstance();
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

        for (auto &attr : desc.attributes) {
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
        VulkanUtils::createBuffer(context, size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vertexBuffer.vkBuffer, vertexBuffer.vkDeviceMemory
        );
        VulkanUtils::updateBufferMemory(context, vertexBuffer.vkDeviceMemory, 0, size, data);
    } else {
        VulkanUtils::createBufferLocal(context, data, size, usage, vertexBuffer.vkBuffer, vertexBuffer.vkDeviceMemory);
    }

    return mVertexBuffers.move(vertexBuffer);
}

RenderDevice::ID VulkanRenderDevice::createIndexBuffer(BufferUsage type, uint32 size, const void *data) {
    VulkanIndexBuffer indexBuffer = { };
    indexBuffer.size = size;
    indexBuffer.usage = type;

    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    if (type == BufferUsage::Dynamic) {
        VulkanUtils::createBuffer(context, size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     indexBuffer.vkBuffer, indexBuffer.vkDeviceMemory
        );
        VulkanUtils::updateBufferMemory(context, indexBuffer.vkDeviceMemory, 0, size, data);
    } else {
        VulkanUtils::createBufferLocal(context, data, size, usage, indexBuffer.vkBuffer, indexBuffer.vkDeviceMemory);
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

    VulkanUtils::updateBufferMemory(context, memory, offset, size, data);
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

    VulkanUtils::updateBufferMemory(context, memory, offset, size, data);
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
            (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {

            // if texture can be sampled in shader and it's a color attachment

            // TODO

        } else if ((usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0 &&
            usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {

            // if texture can be sampled in shader and it's a depth stencil attachment

            // TODO

        } else if ((usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0 &&
                (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {

            // if texture can be sampled in shader and it's not an attachment

            texture.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            // create texture image with mipmaps and allocate memory
            VulkanUtils::createTextureImage(context, textureDesc.data,
                textureDesc.width, textureDesc.height, textureDesc.depth, textureDesc.mipmaps,
                imageType, format, VK_IMAGE_TILING_OPTIMAL,
                texture.image, texture.imageMemory, texture.layout);

            // create image view
            VkImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = textureDesc.mipmaps;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            VulkanUtils::createImageView(context, 
                texture.imageView, texture.image, 
                viewType, format, subresourceRange);

            return mTextureObjects.move(texture);

        } else {

            VulkanException("VulkanRenderDevice::Texture can't be color and depth stencil attachment at the same time");
        }

    } else if (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT &&
        (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {

        // if depth stencil attahment

        VulkanUtils::createDepthStencilBuffer(context, textureDesc.width, textureDesc.height, textureDesc.depth,
            imageType, format, viewType, texture.image, texture.imageMemory, texture.imageView);

        texture.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        return mTextureObjects.move(texture);

    } else if (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT &&
        (usageFlags & (uint32)VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {

        // if color attachment

        // TODO
        texture.image = VK_NULL_HANDLE;
        texture.imageMemory = VK_NULL_HANDLE;
        texture.imageView = VK_NULL_HANDLE;
        texture.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        return mTextureObjects.move(texture);
    } else {
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

RenderDevice::ID VulkanRenderDevice::createFramebufferFormat(const std::vector<RenderDevice::FramebufferAttachmentDesc> &attachments) {
    std::vector<VkAttachmentDescription> attachmentDescriptions(attachments.size());
    std::vector<VkAttachmentReference> attachmentReferences(attachments.size());

    bool useDepthStencil = false;
    VkAttachmentReference depthStencilAttachmentReference;

    for (uint32 i = 0; i < attachments.size(); i++) {
        const RenderDevice::FramebufferAttachmentDesc& attachment = attachments[i];
        VkImageLayout layout = VulkanDefinitions::imageLayout(attachment.type);

        VkAttachmentDescription description = {};
        description.format = VulkanDefinitions::dataFormat(attachment.format);
        description.samples = VulkanDefinitions::samplesCount(attachment.samples);
        description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout = layout;

        VkAttachmentReference reference = {};
        reference.attachment = i;
        reference.layout = layout;

        if (attachment.type == AttachmentType::DepthStencil) {
            if (useDepthStencil) {
                throw VulkanException("An attempt to use more then 1 depth stencil attachment");
            } else {
                useDepthStencil = true;
                depthStencilAttachmentReference = reference;
            }
        } else {
            attachmentReferences.push_back(reference);
        }

        attachmentDescriptions.push_back(description);
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = (uint32) attachmentReferences.size();
    subpass.pColorAttachments = attachmentReferences.data();
    if (useDepthStencil) subpass.pDepthStencilAttachment = &depthStencilAttachmentReference;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32) attachmentDescriptions.size();
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass renderPass;

    if (vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw VulkanException("Failed to create render pass");
    }

    VulkanFrameBufferFormat format = {};
    format.renderPass = renderPass;

    return mFrameBufferFormats.move(format);
}

void VulkanRenderDevice::destroyFramebufferFormat(RenderDevice::ID framebufferFormat) {
    vkDestroyRenderPass(context.device, mFrameBufferFormats.get(framebufferFormat).renderPass, nullptr);
    mFrameBufferFormats.remove(framebufferFormat);
}

RenderDevice::ID VulkanRenderDevice::createFramebuffer(const std::vector<RenderDevice::ID>& attachmentIds,
    RenderDevice::ID framebufferFormatId) {

    // get framebuffer attachemnts info
    std::vector<VkImageView> framebufferAttchViews;
    uint32 width = 0, height = 0;

    for (size_t i = 0; i < attachmentIds.size(); i++) {
        VulkanTextureObject& att = mTextureObjects.get(attachmentIds[i]);

        // also, check widths and heights
        if (i == 0)
        {
            width = att.width;
            height = att.height;
        }
        else if (width != att.width || height != att.height)
        {
            throw VulkanException("Framebuffer attachments must be same size");
        }

        framebufferAttchViews.push_back(att.imageView);
    }

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0;
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;
    framebufferInfo.attachmentCount = framebufferAttchViews.size();
    framebufferInfo.pAttachments = framebufferAttchViews.data();
    // get render pass from framebuffer format
    framebufferInfo.renderPass = mFrameBufferFormats.get(framebufferFormatId).renderPass;

    VkFramebuffer framebuffer;
    VkResult r = vkCreateFramebuffer(context.device, &framebufferInfo, nullptr, &framebuffer);

    if (r != VK_SUCCESS)
    {
        throw VulkanException("Can't create framebuffer");
    }

    return mFrameBuffers.add(framebuffer);
}

void VulkanRenderDevice::destroyFramebuffer(RenderDevice::ID framebufferId) {
    VkFramebuffer framebuffer = mFrameBuffers.get(framebufferId);
    vkDestroyFramebuffer(context.device, framebuffer, nullptr);

    mFrameBuffers.remove(framebufferId);
}

RenderDevice::ID VulkanRenderDevice::createUniformSet(const UniformSetDesc &setDesc, ID uniformLayout) {
/*
    VulkanUniformLayout uniformLayout = {};

    VkDescriptorSetLayout &descriptorSetLayout = uniformLayout.descriptorSetLayout;
    VkDescriptorSet &descriptorSet = uniformLayout.descriptorSet;

    const std::vector<UniformBufferDesc> &uniformBuffers = setDesc.buffers;
    const std::vector<UniformTextureDesc> &uniformTextures = setDesc.textures;

    // get all bindings
    uint32 uniformCount = uniformBuffers.size();
    uint32 texturesCount = uniformTextures.size();

    uint32 bindingsCount = uniformCount + texturesCount;
    std::vector<VkDescriptorSetLayoutBinding> bindings(bindingsCount);

    for (uint32 i = 0; i < uniformCount; i++) {
        bindings[i].binding = uniformBuffers[i].binding;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VulkanDefinitions::shaderStageFlags(uniformBuffers[i].stageFlags);
        bindings[i].pImmutableSamplers = nullptr;
    }

    for (uint32 i = 0; i < texturesCount; i++) {
        uint32 wi = i + uniformCount;

        bindings[wi].binding = uniformTextures[i].binding;
        bindings[wi].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[wi].descriptorCount = 1;
        bindings[wi].stageFlags = VulkanDefinitions::shaderStageFlags(uniformTextures[i].stageFlags);
        bindings[wi].pImmutableSamplers = nullptr;
    }

    // all bindings are in one array, create descriptor set layout
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
    descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutInfo.pNext = nullptr;
    descriptorLayoutInfo.bindingCount = bindings.size();
    descriptorLayoutInfo.pBindings = bindings.data();

    VkResult r = vkCreateDescriptorSetLayout(context.device,
            &descriptorLayoutInfo, nullptr, &descriptorSetLayout);

    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create descriptor set layout");
    }

    // get pool sizes
    std::vector<VkDescriptorPoolSize> poolSizes = {};

    if (uniformCount > 0) {
        VkDescriptorPoolSize uniformsPoolSize;
        uniformsPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformsPoolSize.descriptorCount = uniformCount;

        poolSizes.push_back(uniformsPoolSize);
    }

    if (texturesCount > 0) {
        VkDescriptorPoolSize texturesPoolSize;
        texturesPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texturesPoolSize.descriptorCount = texturesCount;

        poolSizes.push_back(texturesPoolSize);
    }

    VkDescriptorPool pool = context.getDescriptorPool(poolSizes);

    // allocate descriptor set from pool
    VkDescriptorSetAllocateInfo descSetAllocInfo = {};
    descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descSetAllocInfo.pNext = nullptr;
    descSetAllocInfo.descriptorPool = pool;
    descSetAllocInfo.descriptorSetCount = 1;
    descSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    r = vkAllocateDescriptorSets(context.device, &descSetAllocInfo, &descriptorSet);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't allocate descriptor sets from descriptor pool");
    }

    // descriptor set is allocated, so modify it
    std::vector<VkWriteDescriptorSet> writeDescSets(bindingsCount);

    for (uint32 i = 0; i < uniformCount; i++) {
        VkDescriptorBufferInfo bufferInfo;

        // get created uniform buffer
        bufferInfo.buffer = mUniformBuffers.get(uniformBuffers[i].buffer).buffer;
        bufferInfo.offset = uniformBuffers[i].offset;
        bufferInfo.range = uniformBuffers[i].range;

        writeDescSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescSets[i].pNext = nullptr;
        writeDescSets[i].dstSet = descriptorSet;
        writeDescSets[i].dstArrayElement = 0;

        writeDescSets[i].dstBinding = uniformBuffers[i].binding;
        writeDescSets[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescSets[i].descriptorCount = uniformBuffers[i].descriptorCount;
        writeDescSets[i].pBufferInfo = &bufferInfo;
    }

    for (uint32 i = 0; i < texturesCount; i++) {
        uint32 wi = i + uniformCount;

        VkDescriptorImageInfo imageInfo;

        // get created sampler
        imageInfo.sampler = mSamplers.get(uniformTextures[i].sampler);
        const VulkanTextureObject &texture = mTextureObjects.get(uniformTextures[i].texture);
        imageInfo.imageView = texture.imageView;
        imageInfo.imageLayout = texture.layout;

        writeDescSets[wi].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescSets[wi].pNext = nullptr;
        writeDescSets[wi].dstSet = descriptorSet;
        writeDescSets[wi].dstArrayElement = 0;

        writeDescSets[wi].dstBinding = uniformTextures[i].binding;
        writeDescSets[wi].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescSets[wi].descriptorCount = uniformTextures[i].descriptorCount;
        writeDescSets[wi].pImageInfo = &imageInfo;
    }

    vkUpdateDescriptorSets(context.device,
            writeDescSets.size(), writeDescSets.data(), 0, nullptr);

    return mUniformLayouts.move(uniformLayout);
*/
}

void VulkanRenderDevice::destroyUniformSet(RenderDevice::ID layoutId) {

}

RenderDevice::ID VulkanRenderDevice::createUniformBuffer(BufferUsage usage, uint32 size, const void *data) {
    VulkanUniformBuffer uniformBuffer = {};
    uniformBuffer.usage = usage;
    uniformBuffer.size = size;

    if (usage == BufferUsage::Static) {
        VulkanUtils::createBufferLocal(context, data, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                       uniformBuffer.buffer, uniformBuffer.memory);
    } else if (usage == BufferUsage::Dynamic) {
        VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VulkanUtils::createBuffer(context, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  memoryPropertyFlags, uniformBuffer.buffer, uniformBuffer.memory);
    } else {
        throw VulkanException("Undefined uniform buffer usage");
    }

    return mUniformBuffers.move(uniformBuffer);
}

void VulkanRenderDevice::updateUniformBuffer(RenderDevice::ID buffer, uint32 size, uint32 offset, const void *data) {
    const VulkanUniformBuffer &uniformBuffer = mUniformBuffers.get(buffer);

    if (uniformBuffer.usage != BufferUsage::Dynamic) {
        throw VulkanException("Attempt to update static uniform buffer");
    }

    if (offset + size > uniformBuffer.size) {
        throw VulkanException("Attempt to update out-of-buffer memory region for uniform buffer");
    }

    VulkanUtils::updateBufferMemory(context, uniformBuffer.memory, offset, size, data);
}

void VulkanRenderDevice::destroyUniformBuffer(RenderDevice::ID bufferId) {
    VulkanUniformBuffer &uniformBuffer = mUniformBuffers.get(bufferId);

    vkDestroyBuffer(context.device, uniformBuffer.buffer, nullptr);
    vkFreeMemory(context.device, uniformBuffer.memory, nullptr);

    mUniformBuffers.remove(bufferId);
}
