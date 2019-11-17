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
    VulkanVertexBuffer vertexBuffer = {};
    vertexBuffer.size = size;
    vertexBuffer.usage = type;

    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    if (type == BufferUsage::Dynamic) {
        VulkanUtils::createBuffer(context, size, usage,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  vertexBuffer.vkBuffer, vertexBuffer.vkDeviceMemory
        );
        VulkanUtils::updateBufferMemory(context, vertexBuffer.vkDeviceMemory, 0, size, data);
    } else {
        VulkanUtils::createBufferLocal(context, data, size, usage, vertexBuffer.vkBuffer, vertexBuffer.vkDeviceMemory);
    }

    return mVertexBuffers.move(vertexBuffer);
}

RenderDevice::ID VulkanRenderDevice::createIndexBuffer(BufferUsage type, uint32 size, const void *data) {
    VulkanIndexBuffer indexBuffer = {};
    indexBuffer.size = size;
    indexBuffer.usage = type;

    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    if (type == BufferUsage::Dynamic) {
        VulkanUtils::createBuffer(context, size, usage,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  indexBuffer.vkBuffer, indexBuffer.vkDeviceMemory
        );
        VulkanUtils::updateBufferMemory(context, indexBuffer.vkDeviceMemory, 0, size, data);
    } else {
        VulkanUtils::createBufferLocal(context, data, size, usage, indexBuffer.vkBuffer, indexBuffer.vkDeviceMemory);
    }

    return mIndexBuffers.move(indexBuffer);
}

void VulkanRenderDevice::updateVertexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
    const VulkanVertexBuffer &buffer = mVertexBuffers.get(bufferId);
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
    const VulkanIndexBuffer &buffer = mIndexBuffers.get(bufferId);
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
    VulkanVertexBuffer &buffer = mVertexBuffers.get(bufferId);

    vkDestroyBuffer(device, buffer.vkBuffer, nullptr);
    vkFreeMemory(device, buffer.vkDeviceMemory, nullptr);

    mVertexBuffers.remove(bufferId);
}

void VulkanRenderDevice::destroyIndexBuffer(RenderDevice::ID bufferId) {
    const VkDevice &device = context.device;
    VulkanIndexBuffer &buffer = mIndexBuffers.get(bufferId);

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

    if (usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT) {
        if (usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT &&
            (usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {

            // if texture can be sampled in shader and it's a color attachment

            // TODO

        } else if ((usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0 &&
                   usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {

            // if texture can be sampled in shader and it's a depth stencil attachment

            // TODO

        } else if ((usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0 &&
                   (usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {

            // if texture can be sampled in shader and it's not an attachment

            texture.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            // create texture image with mipmaps and allocate memory
            VulkanUtils::createTextureImage(context, textureDesc.data,
                                            textureDesc.width, textureDesc.height, textureDesc.depth,
                                            textureDesc.mipmaps,
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

    } else if (usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT &&
               (usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {

        // if depth stencil attahment

        VulkanUtils::createDepthStencilBuffer(context, textureDesc.width, textureDesc.height, textureDesc.depth,
                                              imageType, format, viewType, texture.image, texture.imageMemory,
                                              texture.imageView);

        texture.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        return mTextureObjects.move(texture);

    } else if (usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT &&
               (usageFlags & (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {

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
        auto &window = *i;
        if (window.name == surfaceName) {
            return i.getID();
        }
    }
    return INVALID;
}

void VulkanRenderDevice::getSurfaceSize(RenderDevice::ID surface, uint32 &width, uint32 &height) {
    auto &window = mSurfaces.get(surface);

    width = window.width;
    height = window.height;
}

RenderDevice::ID
VulkanRenderDevice::createFramebufferFormat(const std::vector<RenderDevice::FramebufferAttachmentDesc> &attachments) {
    std::vector<VkAttachmentDescription> attachmentDescriptions(attachments.size());
    std::vector<VkAttachmentReference> attachmentReferences(attachments.size());

    bool useDepthStencil = false;
    VkAttachmentReference depthStencilAttachmentReference;

    for (uint32 i = 0; i < attachments.size(); i++) {
        const RenderDevice::FramebufferAttachmentDesc &attachment = attachments[i];
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

RenderDevice::ID VulkanRenderDevice::createFramebuffer(const std::vector<RenderDevice::ID> &attachmentIds,
                                                       RenderDevice::ID framebufferFormatId) {

    // get framebuffer attachments info
    std::vector<VkImageView> framebufferAttchViews;
    uint32 width = 0, height = 0;

    for (size_t i = 0; i < attachmentIds.size(); i++) {
        VulkanTextureObject &att = mTextureObjects.get(attachmentIds[i]);

        // also, check widths and heights
        if (i == 0) {
            width = att.width;
            height = att.height;
        } else if (width != att.width || height != att.height) {
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

    if (r != VK_SUCCESS) {
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
    auto &layout = mUniformLayouts.get(uniformLayout);
    auto setLayout = layout.setLayout;

    const auto &uniformBuffers = setDesc.buffers;
    const auto &uniformTextures = setDesc.textures;
    uint32 buffersCount = uniformBuffers.size();
    uint32 texturesCount = uniformTextures.size();

    if (buffersCount != layout.buffersCount || texturesCount != layout.texturesCount) {
        throw VulkanException("Incompatible uniform layout and uniform set descriptor");
    }

    VkResult result;
    VkDescriptorSet descriptorSet;

    if (layout.freeSets.empty()) {
        auto &pool = VulkanUtils::getAvailableDescriptorPool(context, layout);

        VkDescriptorSetAllocateInfo descSetAllocInfo = {};
        descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descSetAllocInfo.pNext = nullptr;
        descSetAllocInfo.descriptorPool = pool.pool;
        descSetAllocInfo.descriptorSetCount = 1;
        descSetAllocInfo.pSetLayouts = &setLayout;

        result = vkAllocateDescriptorSets(context.device, &descSetAllocInfo, &descriptorSet);

        if (result != VK_SUCCESS) {
            throw VulkanException("Can't allocate descriptor set from descriptor pool");
        }

        pool.allocatedSets += 1;
        layout.usedDescriptorSets += 1;
    } else {
        descriptorSet = layout.freeSets.back();
        layout.freeSets.pop_back();
        layout.usedDescriptorSets += 1;
    }

    std::vector<VkWriteDescriptorSet> writeDescSets;
    writeDescSets.reserve(buffersCount + texturesCount);

    std::vector<VkDescriptorBufferInfo> buffersInfo;
    buffersInfo.reserve(buffersCount);

    std::vector<VkDescriptorImageInfo> imagesInfo;
    imagesInfo.reserve(texturesCount);

    for (auto &buffer: uniformBuffers) {
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = mUniformBuffers.get(buffer.buffer).buffer;
        bufferInfo.offset = buffer.offset;
        bufferInfo.range = buffer.range;

        buffersInfo.push_back(bufferInfo);

        VkWriteDescriptorSet writeDescriptor;
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.pNext = nullptr;
        writeDescriptor.dstSet = descriptorSet;
        writeDescriptor.dstArrayElement = 0;
        writeDescriptor.dstBinding = buffer.binding;
        writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.pBufferInfo = &buffersInfo.back();

        writeDescSets.push_back(writeDescriptor);
    }

    for (auto &texture: uniformTextures) {
        const auto &textureObject = mTextureObjects.get(texture.texture);

        VkDescriptorImageInfo imageInfo;
        imageInfo.sampler = mSamplers.get(texture.sampler);
        imageInfo.imageView = textureObject.imageView;
        imageInfo.imageLayout = textureObject.layout;

        imagesInfo.push_back(imageInfo);

        VkWriteDescriptorSet writeDescriptor;
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.pNext = nullptr;
        writeDescriptor.dstSet = descriptorSet;
        writeDescriptor.dstArrayElement = 0;
        writeDescriptor.dstBinding = texture.binding;
        writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.pImageInfo = &imagesInfo.back();
    }

    vkUpdateDescriptorSets(context.device, (uint32) writeDescSets.size(), writeDescSets.data(), 0, nullptr);

    VulkanUniformSet uniformSet = {};
    uniformSet.uniformLayout = uniformLayout;
    uniformSet.descriptorSet = descriptorSet;

    return mUniformSets.move(uniformSet);
}

void VulkanRenderDevice::destroyUniformSet(RenderDevice::ID setId) {
    auto& uniformSet = mUniformSets.get(setId);
    auto& layout = mUniformLayouts.get(uniformSet.uniformLayout);

    layout.usedDescriptorSets -= 1;
    layout.freeSets.push_back(uniformSet.descriptorSet);

    mUniformSets.remove(setId);
}

RenderDevice::ID VulkanRenderDevice::createUniformLayout(const RenderDevice::UniformLayoutDesc &layoutDesc) {
    VkResult result;
    VkDescriptorSetLayout descriptorSetLayout;

    const auto &textures = layoutDesc.textures;
    const auto &buffers = layoutDesc.buffers;
    auto texturesCount = (uint32) layoutDesc.textures.size();
    auto buffersCount = (uint32) layoutDesc.buffers.size();

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(texturesCount + buffersCount);

    for (const auto &texture: textures) {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = texture.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.stageFlags = VulkanDefinitions::shaderStageFlags(texture.flags);
        binding.pImmutableSamplers = nullptr;

        bindings.push_back(binding);
    }

    for (const auto &buffer: buffers) {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = buffer.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.stageFlags = VulkanDefinitions::shaderStageFlags(buffer.flags);
        binding.pImmutableSamplers = nullptr;

        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = (uint32) bindings.size();
    createInfo.pBindings = bindings.data();

    result = vkCreateDescriptorSetLayout(context.device, &createInfo, nullptr, &descriptorSetLayout);

    if (result != VK_SUCCESS) {
        throw VulkanException("Failed to create descriptor set layout");
    }

    VulkanUniformLayout uniformLayout;
    uniformLayout.buffersCount = buffersCount;
    uniformLayout.texturesCount = texturesCount;
    uniformLayout.setLayout = descriptorSetLayout;
    uniformLayout.usedDescriptorSets = 0;

    return mUniformLayouts.move(uniformLayout);
}

void VulkanRenderDevice::destroyUniformLayout(RenderDevice::ID layout) {
    auto &uniformLayout = mUniformLayouts.get(layout);

    // No uniform set must use this layout if we want to destroy it
    if (uniformLayout.usedDescriptorSets != 0) {
        throw VulkanException("An attempt to destroy in-use uniform layout");
    }

    for (auto &pool: uniformLayout.pools) {
        vkDestroyDescriptorPool(context.device, pool.pool, nullptr);
    }

    vkDestroyDescriptorSetLayout(context.device, uniformLayout.setLayout, nullptr);
    mUniformLayouts.remove(layout);
}

RenderDevice::ID VulkanRenderDevice::createUniformBuffer(BufferUsage usage, uint32 size, const void *data) {
    VulkanUniformBuffer uniformBuffer = {};
    uniformBuffer.usage = usage;
    uniformBuffer.size = size;

    if (usage == BufferUsage::Static) {
        VulkanUtils::createBufferLocal(context, data, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                       uniformBuffer.buffer, uniformBuffer.memory);
    } else if (usage == BufferUsage::Dynamic) {
        VkMemoryPropertyFlags memoryPropertyFlags =
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
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

RenderDevice::ID VulkanRenderDevice::createShaderProgram(const std::vector<RenderDevice::ShaderDataDesc> &shaders) {
    VulkanShaderProgram program = {};
    program.shaders.reserve(shaders.size());

    for (const auto& desc: shaders) {
        VkResult result;
        VkShaderModule module;

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pCode = (const uint32*) desc.source.data();
        createInfo.codeSize = (uint32) desc.source.size();

        result = vkCreateShaderModule(context.device, &createInfo, nullptr, &module);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create shader module");
        }

        VulkanShader shader= {};
        shader.module = module;
        shader.shaderStage = VulkanDefinitions::shaderStageBit(desc.type);

        program.shaders.push_back(shader);
    }

    return mShaderPrograms.move(program);
}

void VulkanRenderDevice::destroyShaderProgram(RenderDevice::ID program) {
    auto& vulkanProgram = mShaderPrograms.get(program);

    for (auto& shader: vulkanProgram.shaders) {
        vkDestroyShaderModule(context.device, shader.module, nullptr);
    }

    mShaderPrograms.remove(program);
}