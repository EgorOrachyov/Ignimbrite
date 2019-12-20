//
// Created by Egor Orachyov on 2019-11-02.
//

#include <VulkanRenderDevice.h>
#include <VulkanDefinitions.h>
#include <VulkanUtils.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <array>

namespace ignimbrite {

    VulkanRenderDevice::VulkanRenderDevice(uint32 extensionsCount, const char *const *extensions) {
        context.fillRequiredExt(extensionsCount, extensions);
        context.createInstance();
        context.setupDebugMessenger();
        context.pickPhysicalDevice();
        context.createLogicalDevice();
        context.createCommandPools();
    }

    VulkanRenderDevice::~VulkanRenderDevice() {
        context.destroyCommandPools();
        context.destroyLogicalDevice();
        context.destroyDebugMessenger();
        context.destroyInstance();
    }

    RenderDevice::ID
    VulkanRenderDevice::createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) {
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
            VulkanUtils::createBufferLocal(context, data, size, usage, vertexBuffer.vkBuffer,
                                           vertexBuffer.vkDeviceMemory);
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
            VulkanUtils::createBufferLocal(context, data, size, usage, indexBuffer.vkBuffer,
                                           indexBuffer.vkDeviceMemory);
        }

        return mIndexBuffers.move(indexBuffer);
    }

    void
    VulkanRenderDevice::updateVertexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
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

    void
    VulkanRenderDevice::updateIndexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
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

        mIndexBuffers.remove(bufferId);
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

        auto color = (usageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0;
        auto depth = (usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
        auto sampling = (usageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) != 0;

        // An image could be sampled, therefore it must have shader read layout
        // Otherwise it could not be sampled and must have the following layout: color or depth attachment

        if (sampling) {
            texture.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else if (color) {
            texture.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else if (depth) {
            texture.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else {
            throw VulkanException("Texture has invalid usage flags");
        }

        if (color) {

            VulkanUtils::createImage(
                    context,
                    textureDesc.width, textureDesc.height, textureDesc.depth,
                    1, imageType, format, VK_IMAGE_TILING_OPTIMAL, usageFlags,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    texture.image, texture.imageMemory
            );

            VkImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            VkComponentMapping components = {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY
            };

            VulkanUtils::createImageView(
                    context,
                    texture.imageView, texture.image,
                    viewType, format, subresourceRange, components
            );

        } else if (depth) {

            VulkanUtils::createDepthStencilBuffer(
                    context,
                    textureDesc.width, textureDesc.height, textureDesc.depth,
                    imageType, format, viewType,
                    texture.image, texture.imageMemory,
                    texture.imageView, usageFlags
            );

            VkImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            subresourceRange.baseMipLevel = 0; // depth stencil doesn't have mipmaps
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            VkComponentMapping components = {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY
            };

            VulkanUtils::createImageView(
                    context,
                    texture.imageView, texture.image,
                    viewType, format, subresourceRange, components
            );

        } else if (sampling) {

            // create texture image with mipmaps and allocate memory
            VulkanUtils::createTextureImage(
                    context, textureDesc.data,
                    textureDesc.width, textureDesc.height, textureDesc.depth,
                    textureDesc.mipmaps,
                    imageType, format, VK_IMAGE_TILING_OPTIMAL,
                    texture.image, texture.imageMemory, texture.layout
            );

            VkImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = textureDesc.mipmaps;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            VkComponentMapping components = {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY
            };

            VulkanUtils::createImageView(
                    context,
                    texture.imageView, texture.image,
                    viewType, format, subresourceRange, components
            );

        }

        return mTextureObjects.move(texture);
    }

    void VulkanRenderDevice::destroyTexture(RenderDevice::ID textureId) {
        auto &device = context.device;
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
        VkResult result = vkCreateSampler(context.device, &samplerInfo, nullptr, &sampler);

        if (result != VK_SUCCESS) {
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
    VulkanRenderDevice::createFramebufferFormat(
            const std::vector<RenderDevice::FramebufferAttachmentDesc> &attachments) {
        std::vector<VkAttachmentDescription> attachmentDescriptions;
        attachmentDescriptions.reserve(attachments.size());

        std::vector<VkAttachmentReference> attachmentReferences;
        attachmentReferences.reserve(attachments.size());

        bool useDepthStencil = false;
        VkAttachmentReference depthStencilAttachmentReference;

        for (uint32 i = 0; i < attachments.size(); i++) {
            const auto &attachment = attachments[i];
            auto layout = VulkanDefinitions::imageLayout(attachment.type);

            VkAttachmentDescription description = {};
            description.format = VulkanDefinitions::dataFormat(attachment.format);
            description.samples = VulkanDefinitions::samplesCount(attachment.samples);
            description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // it is default layout for any texture (except present image)

            VkAttachmentReference reference = {};
            reference.attachment = i;
            reference.layout = layout;

            if (attachment.type == AttachmentType::DepthStencil) {
                if (useDepthStencil) {
                    throw VulkanException("An attempt to use more than 1 depth stencil attachment");
                } else {
                    useDepthStencil = true;
                    depthStencilAttachmentReference = reference;
                }
            } else {
                attachmentReferences.push_back(reference);
            }

            attachmentDescriptions.push_back(description);
        }

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = (uint32) attachmentReferences.size();
        subpass.pColorAttachments = attachmentReferences.data();
        subpass.pDepthStencilAttachment = useDepthStencil ? &depthStencilAttachmentReference : nullptr;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = (uint32) attachmentDescriptions.size();
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = (uint32) dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();

        VkResult result;
        VkRenderPass renderPass;

        result = vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &renderPass);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create render pass");
        }

        VulkanFrameBufferFormat format = {};
        format.renderPass = renderPass;
        format.useDepthStencil = useDepthStencil;
        format.numOfAttachments = (uint32) attachmentDescriptions.size();

        return mFrameBufferFormats.move(format);
    }

    void VulkanRenderDevice::destroyFramebufferFormat(RenderDevice::ID framebufferFormat) {
        auto &format = mFrameBufferFormats.get(framebufferFormat);
        vkDestroyRenderPass(context.device, format.renderPass, nullptr);

        mFrameBufferFormats.remove(framebufferFormat);
    }

    RenderDevice::ID VulkanRenderDevice::createFramebuffer(const std::vector<RenderDevice::ID> &attachmentIds,
                                                           RenderDevice::ID framebufferFormatId) {
        VulkanFrameBuffer fbo = {};

        if (attachmentIds.empty()) {
            throw VulkanException("An attempt to create empty frame buffer");
        }

        auto &format = mFrameBufferFormats.get(framebufferFormatId);

        if (attachmentIds.size() != format.numOfAttachments) {
            throw VulkanException("Attachments count is incompatible with framebuffer format");
        }

        std::vector<VkImageView> attachments;
        attachments.reserve(attachmentIds.size());

        auto &base = mTextureObjects.get(attachmentIds[0]);
        uint32 width = base.width, height = base.height;

        for (auto &id: attachmentIds) {
            auto &texture = mTextureObjects.get(id);

            if (texture.width != width || texture.height != height) {
                throw VulkanException("Framebuffer attachments must be of the same size");
            }

            attachments.push_back(texture.imageView);
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = nullptr;
        framebufferInfo.flags = 0;
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;
        framebufferInfo.attachmentCount = (uint32) attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.renderPass = format.renderPass;

        VkFramebuffer framebuffer;
        VkResult result = vkCreateFramebuffer(context.device, &framebufferInfo, nullptr, &framebuffer);

        if (result != VK_SUCCESS) {
            throw VulkanException("Filed to create framebuffer");
        }

        fbo.framebuffer = framebuffer;
        fbo.framebufferFormatId = framebufferFormatId;
        fbo.width = width;
        fbo.height = height;

        return mFrameBuffers.move(fbo);
    }

    void VulkanRenderDevice::destroyFramebuffer(RenderDevice::ID framebufferId) {
        VulkanFrameBuffer fbo = mFrameBuffers.get(framebufferId);
        vkDestroyFramebuffer(context.device, fbo.framebuffer, nullptr);

        mFrameBuffers.remove(framebufferId);
    }

    RenderDevice::ID VulkanRenderDevice::createUniformSet(const UniformSetDesc &setDesc, ID uniformLayout) {
        auto &layout = mUniformLayouts.get(uniformLayout);

        const auto &uniformBuffers = setDesc.buffers;
        const auto &uniformTextures = setDesc.textures;
        uint32 buffersCount = uniformBuffers.size();
        uint32 texturesCount = uniformTextures.size();

        if (buffersCount != layout.buffersCount || texturesCount != layout.texturesCount) {
            throw VulkanException("Incompatible uniform layout and uniform set descriptor");
        }

        if (layout.buffersCount == 0 && layout.texturesCount == 0) {
            throw VulkanException("Uniform layout has not textures and buffers to be bounded");
        }

        VkDescriptorSet descriptorSet = VulkanUtils::getAvailableDescriptorSet(context, layout);

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

            writeDescSets.push_back(writeDescriptor);
        }

        vkUpdateDescriptorSets(context.device, (uint32) writeDescSets.size(), writeDescSets.data(), 0, nullptr);

        VulkanUniformSet uniformSet = {};
        uniformSet.uniformLayout = uniformLayout;
        uniformSet.descriptorSet = descriptorSet;

        return mUniformSets.move(uniformSet);
    }

    void VulkanRenderDevice::destroyUniformSet(RenderDevice::ID setId) {
        auto &uniformSet = mUniformSets.get(setId);
        auto &layout = mUniformLayouts.get(uniformSet.uniformLayout);

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
            VulkanUtils::updateBufferMemory(context, uniformBuffer.memory, 0, size, data);
        } else {
            throw VulkanException("Undefined uniform buffer usage");
        }

        return mUniformBuffers.move(uniformBuffer);
    }

    void
    VulkanRenderDevice::updateUniformBuffer(RenderDevice::ID buffer, uint32 size, uint32 offset, const void *data) {
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

        for (const auto &desc: shaders) {

            if (desc.language != ShaderLanguage::SPIRV) {
                throw VulkanException("Compiling shaders from not SPIR-V languages is not supported");
            }

            VkResult result;
            VkShaderModule module;

            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.pCode = (const uint32 *) desc.source.data();
            createInfo.codeSize = (uint32) desc.source.size();

            result = vkCreateShaderModule(context.device, &createInfo, nullptr, &module);

            if (result != VK_SUCCESS) {
                throw VulkanException("Failed to create shader module");
            }

            VulkanShader shader = {};
            shader.module = module;
            shader.shaderStage = VulkanDefinitions::shaderStageBit(desc.type);

            program.shaders.push_back(shader);
        }

        return mShaderPrograms.move(program);
    }

    void VulkanRenderDevice::destroyShaderProgram(RenderDevice::ID program) {
        auto &vulkanProgram = mShaderPrograms.get(program);

        for (auto &shader: vulkanProgram.shaders) {
            vkDestroyShaderModule(context.device, shader.module, nullptr);
        }

        mShaderPrograms.remove(program);
    }

    RenderDevice::ID VulkanRenderDevice::createGraphicsPipeline(PrimitiveTopology topology, RenderDevice::ID program,
                                                                RenderDevice::ID vertexLayout,
                                                                RenderDevice::ID uniformLayout,
                                                                RenderDevice::ID framebufferFormat,
                                                                const RenderDevice::PipelineRasterizationDesc &rasterizationDesc,
                                                                const RenderDevice::PipelineBlendStateDesc &blendStateDesc,
                                                                const RenderDevice::PipelineDepthStencilStateDesc &depthStencilStateDesc) {
        const auto &vkProgram = mShaderPrograms.get(program);
        const auto &vkUniformLayout = mUniformLayouts.get(uniformLayout);
        const auto &vkVertexLayout = mVertexLayouts.get(vertexLayout);
        const auto &vkFramebufferFormat = mFrameBufferFormats.get(framebufferFormat);

        auto framebufferColorAttachmentsCount = vkFramebufferFormat.useDepthStencil ?
                                                vkFramebufferFormat.numOfAttachments -
                                                1
                                                                                    : vkFramebufferFormat.numOfAttachments;

        if (blendStateDesc.attachments.size() != framebufferColorAttachmentsCount) {
            throw VulkanException(
                    "Incompatible number of color and blend attachments for specified framebuffer format and blend state");
        }

        if (depthStencilStateDesc.depthTestEnable && !vkFramebufferFormat.useDepthStencil) {
            throw VulkanException("Specified framebuffer format does not support depth/stencil buffer usage");
        }

        VkResult result;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        VulkanUtils::createPipelineLayout(context, vkUniformLayout, pipelineLayout);

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(vkProgram.shaders.size());

        for (const auto &shader: vkProgram.shaders) {
            VkPipelineShaderStageCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            createInfo.stage = shader.shaderStage;
            createInfo.module = shader.module;
            createInfo.pName = "main";
            createInfo.pSpecializationInfo = nullptr;

            shaderStages.push_back(createInfo);
        }

        VkPipelineVertexInputStateCreateInfo vertexInput = {};
        VulkanUtils::createVertexInputState(vkVertexLayout, vertexInput);

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        VulkanUtils::createInputAssembly(topology, inputAssembly);

        VkViewport viewport = {};
        VkRect2D scissor = {};
        VkPipelineViewportStateCreateInfo viewportState = {};
        VulkanUtils::createViewportState(viewport, scissor, viewportState);

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        VulkanUtils::createRasterizationState(rasterizationDesc, rasterizer);

        VkDynamicState states[] = {
                VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
                VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
                VkDynamicState::VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = states;
        dynamicState.dynamicStateCount = sizeof(states) / sizeof(VkDynamicState);

        VkPipelineMultisampleStateCreateInfo multisampleState = {};
        VulkanUtils::createMultisampleState(multisampleState);

        std::vector<VkPipelineColorBlendAttachmentState> attachments(blendStateDesc.attachments.size());
        for (uint32 i = 0; i < attachments.size(); i++) {
            VulkanUtils::createColorBlendAttachmentState(blendStateDesc.attachments[i], attachments[i]);
        }

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        VulkanUtils::createColorBlendState(blendStateDesc, (uint32) attachments.size(), attachments.data(),
                                           colorBlending);

        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        if (depthStencilStateDesc.depthTestEnable) {
            VulkanUtils::createDepthStencilState(depthStencilStateDesc, depthStencilState);
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = (uint32) shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampleState;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = depthStencilStateDesc.depthTestEnable ? &depthStencilState : nullptr;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = vkFramebufferFormat.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        result = vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create graphics pipeline");
        }

        VulkanGraphicsPipeline graphicsPipeline;
        graphicsPipeline.withSurfaceOnly = false;
        graphicsPipeline.vertexLayout = vertexLayout;
        graphicsPipeline.uniformLayout = uniformLayout;
        graphicsPipeline.framebufferFormat = framebufferFormat;
        graphicsPipeline.program = program;
        graphicsPipeline.pipeline = pipeline;
        graphicsPipeline.pipelineLayout = pipelineLayout;

        return mGraphicsPipelines.move(graphicsPipeline);
    }

    RenderDevice::ID VulkanRenderDevice::createGraphicsPipeline(RenderDevice::ID surface, PrimitiveTopology topology,
                                                                RenderDevice::ID program, RenderDevice::ID vertexLayout,
                                                                RenderDevice::ID uniformLayout,
                                                                const RenderDevice::PipelineRasterizationDesc &rasterizationDesc,
                                                                const RenderDevice::PipelineSurfaceBlendStateDesc &blendStateDesc) {

        const auto &vkProgram = mShaderPrograms.get(program);
        const auto &vkUniformLayout = mUniformLayouts.get(uniformLayout);
        const auto &vkVertexLayout = mVertexLayouts.get(vertexLayout);
        auto &vkSurface = mSurfaces.get(surface);
        auto &vkFramebufferFormat = vkSurface.framebufferFormat;

        VkResult result;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        VulkanUtils::createPipelineLayout(context, vkUniformLayout, pipelineLayout);

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(vkProgram.shaders.size());

        for (const auto &shader: vkProgram.shaders) {
            VkPipelineShaderStageCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            createInfo.stage = shader.shaderStage;
            createInfo.module = shader.module;
            createInfo.pName = "main";
            createInfo.pSpecializationInfo = nullptr;

            shaderStages.push_back(createInfo);
        }

        VkPipelineVertexInputStateCreateInfo vertexInput = {};
        VulkanUtils::createVertexInputState(vkVertexLayout, vertexInput);

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        VulkanUtils::createInputAssembly(topology, inputAssembly);

        VkViewport viewport = {};
        VkRect2D scissor = {};
        VkPipelineViewportStateCreateInfo viewportState = {};
        VulkanUtils::createViewportState(viewport, scissor, viewportState);

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        VulkanUtils::createRasterizationState(rasterizationDesc, rasterizer);

        VkDynamicState states[] = {
                VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
                VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
                VkDynamicState::VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = states;
        dynamicState.dynamicStateCount = sizeof(states) / sizeof(VkDynamicState);

        VkPipelineMultisampleStateCreateInfo multisampleState = {};
        VulkanUtils::createMultisampleState(multisampleState);

        VkPipelineColorBlendAttachmentState attachment;
        VulkanUtils::createColorBlendAttachmentState(blendStateDesc.attachment, attachment);

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        VulkanUtils::createSurfaceColorBlendState(blendStateDesc, &attachment, colorBlending);

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = (uint32) shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampleState;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = vkFramebufferFormat.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        result = vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create graphics pipeline");
        }

        VulkanGraphicsPipeline graphicsPipeline;
        graphicsPipeline.withSurfaceOnly = false;
        graphicsPipeline.vertexLayout = vertexLayout;
        graphicsPipeline.uniformLayout = uniformLayout;
        graphicsPipeline.program = program;
        graphicsPipeline.pipeline = pipeline;
        graphicsPipeline.pipelineLayout = pipelineLayout;
        graphicsPipeline.withSurfaceOnly = true;
        graphicsPipeline.surface = surface;

        return mGraphicsPipelines.move(graphicsPipeline);
    }

    void VulkanRenderDevice::destroyGraphicsPipeline(RenderDevice::ID pipeline) {
        auto &vulkanPipeline = mGraphicsPipelines.get(pipeline);

        vkDestroyPipeline(context.device, vulkanPipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(context.device, vulkanPipeline.pipelineLayout, nullptr);

        mGraphicsPipelines.remove(pipeline);
    }

    void VulkanRenderDevice::drawListBegin() {
        // get free command buffer and reset drawList

        // TODO: get available cmd buffer
//    VkCommandBuffer cmd = ?;
//
//    // clear command buffer
//    vkResetCommandBuffer(cmd, 0);
//
//    // and start recording
//    VkCommandBufferBeginInfo beginInfo = {};
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    vkBeginCommandBuffer(cmd, &beginInfo);

        // for now, just create temp cmd buffer
        VkCommandBuffer cmd = VulkanUtils::beginTempCommandBuffer(context, context.graphicsTempCommandPool);

        // clear draw list
        drawList = {};
        drawList.buffer = cmd;
    }

    void VulkanRenderDevice::drawListEnd() {
        // submit for rendering and wait

        VulkanSurface &surface = mSurfaces.get(drawList.surfaceId);
        uint32 imageIndex = surface.currentImageIndex;
        uint32 frameIndex = surface.currentFrameIndex;
        VkCommandBuffer cmd = drawList.buffer;

        // end renderpass associated with surface
        vkCmdEndRenderPass(cmd);

        // TODO: remove after 'getAvailableCmdBuffer' implemetation
        // as temp cmd buffer is used now, so delete it
        vkEndCommandBuffer(cmd);;
        if (surface.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(context.device, 1, &surface.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }

        surface.imagesInFlight[imageIndex] = surface.inFlightFences[frameIndex];

        // when final colors are output of pipeline
        VkPipelineStageFlags pipelineStageFlags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.pWaitDstStageMask = pipelineStageFlags;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &surface.imageAvailableSemaphores[frameIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &surface.renderFinishedSemaphores[frameIndex];
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        // reset fence to unsignaled state before submitting
        VkResult result = vkResetFences(context.device, 1, &surface.inFlightFences[frameIndex]);
        if (result != VK_SUCCESS) {
            throw VulkanException("Can't reset fence");
        }

        // submit command buffer to queue
        result = vkQueueSubmit(surface.graphicsQueue, 1, &submitInfo, surface.inFlightFences[frameIndex]);
        if (result != VK_SUCCESS) {
            throw VulkanException("Can't submit queue");
        }

        // TODO: remove wait idle
        vkQueueWaitIdle(surface.graphicsQueue);
        vkFreeCommandBuffers(context.device, context.graphicsTempCommandPool, 1, &cmd);
    }

    void VulkanRenderDevice::drawListBindPipeline(RenderDevice::ID graphicsPipelineId) {
        const auto &graphicsPipeline = mGraphicsPipelines.get(graphicsPipelineId);
        vkCmdBindPipeline(drawList.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipeline);
        drawList.pipelineLayout = graphicsPipeline.pipelineLayout;
        drawList.pipelineAttached = true;
    }

    void VulkanRenderDevice::drawListBindUniformSet(RenderDevice::ID uniformSetId) {
        const auto &uniformSet = mUniformSets.get(uniformSetId);
        vkCmdBindDescriptorSets(drawList.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawList.pipelineLayout, 0, 1,
                                &uniformSet.descriptorSet, 0, nullptr);

        drawList.uniformSetAttached = true;
    }

    void VulkanRenderDevice::drawListBindIndexBuffer(RenderDevice::ID indexBufferId,
                                                     IndicesType indicesType, uint32 offset) {
        const auto &indexBuffer = mIndexBuffers.get(indexBufferId);
        vkCmdBindIndexBuffer(drawList.buffer, indexBuffer.vkBuffer, offset, VulkanDefinitions::indexType(indicesType));

        drawList.indexBufferAttached = true;
    }

    void VulkanRenderDevice::drawListBindVertexBuffer(RenderDevice::ID vertexBufferId,
                                                      uint32 binding, uint32 offset) {
        const auto &vertexBuffer = mVertexBuffers.get(vertexBufferId);
        VkDeviceSize offsets[1] = {offset};
        vkCmdBindVertexBuffers(drawList.buffer, binding, 1, &vertexBuffer.vkBuffer, offsets);

        drawList.vertexBufferAttached = true;
    }

    void VulkanRenderDevice::drawListDrawIndexed(uint32 indicesCount, uint32 instancesCount) {
        vkCmdDrawIndexed(drawList.buffer, indicesCount, instancesCount, 0, 0, 0);
        drawList.drawCalled = true;
    }

    void VulkanRenderDevice::drawListDraw(uint32 verticesCount, uint32 instancesCount) {
        vkCmdDraw(drawList.buffer, verticesCount, instancesCount, 0, 0);
        drawList.drawCalled = true;
    }

// TODO: DELETE THIS
    static bool firstTime = true;

    void VulkanRenderDevice::swapBuffers(RenderDevice::ID surfaceId) {
        VulkanSurface &surface = mSurfaces.get(surfaceId);
        const uint32 imageIndex = surface.currentImageIndex;
        const uint32 frameIndex = surface.currentFrameIndex;
        VkResult result;

        if (!firstTime) {
            // queue image presentation
            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pNext = nullptr;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &surface.swapChain;
            presentInfo.pImageIndices = &imageIndex;
            // wait until render will be finished
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &surface.renderFinishedSemaphores[frameIndex];
            presentInfo.pResults = nullptr;

            result = vkQueuePresentKHR(surface.presentQueue, &presentInfo);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                // TODO: resize
            } else if (result != VK_SUCCESS) {
                throw VulkanException("Can't queue and image for presentation");
            }
        }
        firstTime = false;
        uint32 nextFrameIndex = (frameIndex + 1) % surface.maxFramesInFlight;

        // NOTE: this block can be in the beginning of frame draw
        {
            // wait when next will be done, i.e. fence will be in signaled state
            vkWaitForFences(context.device, 1, &surface.inFlightFences[nextFrameIndex], VK_TRUE, UINT64_MAX);

            // acquire next image
            uint32 nextImageIndex;
            result = vkAcquireNextImageKHR(context.device, surface.swapChain, UINT64_MAX,
                                           surface.imageAvailableSemaphores[nextFrameIndex], VK_NULL_HANDLE,
                                           &nextImageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                // TODO: process resize
            } else if (result != VK_SUCCESS) {
                throw VulkanException("Can't acquire next image in swapchain");
            }

            surface.currentFrameIndex = nextFrameIndex;
            surface.currentImageIndex = nextImageIndex;
        }
    }

    void VulkanRenderDevice::drawListBindSurface(RenderDevice::ID surfaceId, const RenderDevice::Color &color,
                                                 const RenderDevice::Region &area) {
        // end previous render pass, if exists
        if (drawList.frameBufferAttached) {
            vkCmdEndRenderPass(drawList.buffer);
        }

        VulkanSurface &surface = mSurfaces.get(surfaceId);
        const float clearDepth = 1.0f;
        const uint32 clearStencil = 0;

        VkCommandBuffer cmd = drawList.buffer;
        VkFramebuffer surfFramebuffer = surface.swapChainFramebuffers[surface.currentImageIndex];

        uint32_t viewportOffsetX = area.xOffset;
        uint32_t viewportOffsetY = area.yOffset;
        uint32_t viewportWidth = area.extent.x;
        uint32_t viewportHeight = area.extent.y;

        VkClearValue clearValues[2];
        clearValues[0].color = {{color.components[0],
                                        color.components[1],
                                        color.components[2],
                                        color.components[3]}};
        clearValues[1].depthStencil.depth = clearDepth;
        clearValues[1].depthStencil.stencil = clearStencil;

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = surface.framebufferFormat.renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;

        renderPassBeginInfo.renderArea.extent.width = surface.widthFramebuffer;
        renderPassBeginInfo.renderArea.extent.height = surface.heightFramebuffer;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.framebuffer = surfFramebuffer;

        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport;
        viewport.x = viewportOffsetX;
        viewport.y = viewportOffsetY;
        viewport.width = viewportWidth;
        viewport.height = viewportHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor;
        scissor.extent.width = viewportWidth;
        scissor.extent.height = viewportHeight;
        scissor.offset.x = viewportOffsetX;
        scissor.offset.y = viewportOffsetY;
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        drawList.surfaceId = surfaceId;
    }

    void VulkanRenderDevice::drawListBindFramebuffer(RenderDevice::ID framebufferId, const std::vector<Color> &colors,
                                                     float32 clearDepth, uint32 clearStencil,
                                                     const RenderDevice::Region &area) {
        // end previous render pass, if exists
        if (drawList.frameBufferAttached) {
            vkCmdEndRenderPass(drawList.buffer);
        }

        VulkanFrameBuffer &fbo = mFrameBuffers.get(framebufferId);
        VulkanFrameBufferFormat &fboFormat = mFrameBufferFormats.get(fbo.framebufferFormatId);
        VkCommandBuffer cmd = drawList.buffer;

        uint32 viewportOffsetX = area.xOffset;
        uint32 viewportOffsetY = area.yOffset;
        uint32 viewportWidth = area.extent.x;
        uint32 viewportHeight = area.extent.y;

        // to prevent allocation std::vector on each call of this function ??
        std::vector<VkClearValue> &clearValues = context.tempClearValues;
        if (clearValues.size() < colors.size() + 1) {
            clearValues.resize(colors.size() + 1);
        }

        for (size_t i = 0; i < colors.size(); i++) {
            clearValues[i].color = {
                {colors[i].components[0],
                 colors[i].components[1],
                 colors[i].components[2],
                 colors[i].components[3]}
            };
        }

        {
            VkClearValue depthStencilClearValues = {};
            depthStencilClearValues.depthStencil = { clearDepth, clearStencil };
            clearValues.push_back(depthStencilClearValues);
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = fboFormat.renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;

        renderPassBeginInfo.renderArea.extent.width = fbo.width;
        renderPassBeginInfo.renderArea.extent.height = fbo.height;
        renderPassBeginInfo.clearValueCount = (uint32) clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();
        renderPassBeginInfo.framebuffer = fbo.framebuffer;

        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport;
        viewport.x = viewportOffsetX;
        viewport.y = viewportOffsetY;
        viewport.width = viewportWidth;
        viewport.height = viewportHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor;
        scissor.extent.width = viewportWidth;
        scissor.extent.height = viewportHeight;
        scissor.offset.x = viewportOffsetX;
        scissor.offset.y = viewportOffsetY;
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        drawList.frameBufferAttached = true;
    }

    void VulkanRenderDevice::drawListBindFramebuffer(RenderDevice::ID framebufferId, const std::vector<Color> &colors,
                                                     const RenderDevice::Region &area) {
        drawListBindFramebuffer(framebufferId, colors, 1.0f, 0, area);
    }

} // namespace ignimbrite