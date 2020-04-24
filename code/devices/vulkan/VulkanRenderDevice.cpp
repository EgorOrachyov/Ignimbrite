/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <VulkanRenderDevice.h>
#include <VulkanDefinitions.h>
#include <VulkanUtils.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <array>

namespace ignimbrite {

    using VertexLayout = IRenderDevice::VertexLayout;
    using VertexBuffer = IRenderDevice::VertexBuffer;
    using IndexBuffer = IRenderDevice::IndexBuffer;
    using UniformBuffer = IRenderDevice::UniformBuffer;
    using UniformLayout = IRenderDevice::UniformLayout;
    using UniformSet = IRenderDevice::UniformSet;
    using ShaderProgram = IRenderDevice::ShaderProgram;
    using GraphicsPipeline = IRenderDevice::GraphicsPipeline;
    using FramebufferFormat = IRenderDevice::FramebufferFormat;
    using Framebuffer = IRenderDevice::Framebuffer;
    using Surface = IRenderDevice::Surface;
    using Texture = IRenderDevice::Texture;
    using Sampler = IRenderDevice::Sampler;

    VulkanRenderDevice::VulkanRenderDevice(uint32 extensionsCount, const char *const *extensions) {
        mContext.fillRequiredExt(extensionsCount, extensions);
        mContext.createInstance();
        mContext.setupDebugMessenger();
        mContext.pickPhysicalDevice();
        mContext.createLogicalDevice();
        mContext.createAllocator();
        mContext.createCommandPools();

        VulkanUtils::getSupportedFormats(mSupportedTextureDataFormats);
    }

    VulkanRenderDevice::~VulkanRenderDevice() {
        mContext.destroyCommandPools();
        mContext.destroyAllocator();
        mContext.destroyLogicalDevice();
        mContext.destroyDebugMessenger();
        mContext.destroyInstance();
    }

    ID<VertexLayout> VulkanRenderDevice::createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) {
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

    void VulkanRenderDevice::destroyVertexLayout(ID<VertexLayout> layout) {
        mVertexLayouts.remove(layout);
    }

    ID<VertexBuffer> VulkanRenderDevice::createVertexBuffer(BufferUsage type, uint32 size, const void *data) {
        VulkanVertexBuffer vertexBuffer = {};
        vertexBuffer.size = size;
        vertexBuffer.usage = type;

        VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (type == BufferUsage::Dynamic) {
            VulkanUtils::createBuffer(size,
                                      usage,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      vertexBuffer.vkBuffer,
                                      vertexBuffer.allocation
            );
            if (data != nullptr) {
                VulkanUtils::updateBufferMemory(vertexBuffer.allocation, 0, size, data);
            }
        } else {
            VulkanUtils::createBufferLocal(
                    data,
                    size,
                    usage,
                    vertexBuffer.vkBuffer,
                    vertexBuffer.allocation
            );
        }

        return mVertexBuffers.move(vertexBuffer);
    }

    ID<IndexBuffer> VulkanRenderDevice::createIndexBuffer(BufferUsage type, uint32 size, const void *data) {
        VulkanIndexBuffer indexBuffer = {};
        indexBuffer.size = size;
        indexBuffer.usage = type;

        VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (type == BufferUsage::Dynamic) {
            VulkanUtils::createBuffer(size,
                                      usage,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      indexBuffer.vkBuffer,
                                      indexBuffer.allocation
            );
            if (data != nullptr) {
                VulkanUtils::updateBufferMemory(indexBuffer.allocation, 0, size, data);
            }
        } else {
            VulkanUtils::createBufferLocal(data, size, usage, indexBuffer.vkBuffer,
                                           indexBuffer.allocation);
        }

        return mIndexBuffers.move(indexBuffer);
    }

    void VulkanRenderDevice::updateVertexBuffer(ID<VertexBuffer> bufferId, uint32 size, uint32 offset, const void *data) {
        const VulkanVertexBuffer &buffer = mVertexBuffers.get(bufferId);

        if (buffer.usage != BufferUsage::Dynamic) {
            throw VulkanException("Attempt to update static vertex buffer");
        }

        if (size + offset > buffer.size) {
            throw VulkanException("Attempt to update out-of-buffer memory region for vertex buffer");
        }

        VulkanUtils::updateBufferMemory(buffer.allocation, offset, size, data);
    }

    void VulkanRenderDevice::updateIndexBuffer(ID<IndexBuffer> bufferId, uint32 size, uint32 offset, const void *data) {
        const VulkanIndexBuffer &buffer = mIndexBuffers.get(bufferId);

        if (buffer.usage != BufferUsage::Dynamic) {
            throw VulkanException("Attempt to update static index buffer");
        }

        if (size + offset > buffer.size) {
            throw VulkanException("Attempt to update out-of-buffer memory region for index buffer");
        }

        VulkanUtils::updateBufferMemory(buffer.allocation, offset, size, data);
    }

    void VulkanRenderDevice::destroyVertexBuffer(ID<VertexBuffer> bufferId) {
        VulkanVertexBuffer &buffer = mVertexBuffers.get(bufferId);
        VulkanUtils::destroyBuffer(buffer.vkBuffer, buffer.allocation);

        mVertexBuffers.remove(bufferId);
    }

    void VulkanRenderDevice::destroyIndexBuffer(ID<IndexBuffer> bufferId) {
        VulkanIndexBuffer &buffer = mIndexBuffers.get(bufferId);
        VulkanUtils::destroyBuffer(buffer.vkBuffer, buffer.allocation);

        mIndexBuffers.remove(bufferId);
    }

    ID<Texture> VulkanRenderDevice::createTexture(const IRenderDevice::TextureDesc &textureDesc) {
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
            texture.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        } else {
            throw VulkanException("Texture has invalid usage flags");
        }

        if (color) {

            VulkanUtils::createImage(
                    textureDesc.width, textureDesc.height, textureDesc.depth,
                    1, imageType, format, VK_IMAGE_TILING_OPTIMAL,
                    usageFlags | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    texture.image, texture.allocation
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
                    texture.imageView, texture.image,
                    viewType, format, subresourceRange, components
            );

        } else if (depth) {

            VulkanUtils::createDepthStencilBuffer(
                    textureDesc.width, textureDesc.height, textureDesc.depth,
                    imageType, format, //viewType,
                    texture.image, texture.allocation,
                    usageFlags | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            );

            auto depthOnly = (textureDesc.usageFlags & (uint32) TextureUsageBit::DepthAttachment) != 0x0;

            VkImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = depthOnly ? VK_IMAGE_ASPECT_DEPTH_BIT : (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
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
                    texture.imageView, texture.image,
                    viewType, format, subresourceRange, components
            );

        } else {

            // create texture image with mipmaps and allocate memory
            VulkanUtils::createTextureImage(
                    textureDesc.data, textureDesc.size,
                    textureDesc.width, textureDesc.height, textureDesc.depth,
                    textureDesc.mipmaps,
                    imageType, format, VK_IMAGE_TILING_OPTIMAL,
                    texture.image, texture.allocation, texture.layout
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
                    texture.imageView, texture.image,
                    viewType, format, subresourceRange, components
            );

        }

        return mTextureObjects.move(texture);
    }

    void VulkanRenderDevice::destroyTexture(ID<Texture> textureId) {
        auto &device = mContext.device;
        VulkanTextureObject &imo = mTextureObjects.get(textureId);

        vkDestroyImageView(device, imo.imageView, nullptr);
        VulkanUtils::destroyImage(imo.image, imo.allocation);

        mTextureObjects.remove(textureId);
    }

    ID<Sampler> VulkanRenderDevice::createSampler(const IRenderDevice::SamplerDesc &samplerDesc) {
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
        VkResult result = vkCreateSampler(mContext.device, &samplerInfo, nullptr, &sampler);
        VK_RESULT_ASSERT(result, "Failed to create sampler object");

        return mSamplers.add(sampler);
    }

    void VulkanRenderDevice::destroySampler(ID<Sampler> samplerId) {
        vkDestroySampler(mContext.device, mSamplers.get(samplerId), nullptr);
        mSamplers.remove(samplerId);
    }

    ID<FramebufferFormat> VulkanRenderDevice::createFramebufferFormat(const std::vector<IRenderDevice::FramebufferAttachmentDesc> &attachments) {
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

        std::array<VkSubpassDependency, 2> dependencies = {};

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

        result = vkCreateRenderPass(mContext.device, &renderPassInfo, nullptr, &renderPass);
        VK_RESULT_ASSERT(result, "Failed to create render pass");

        VulkanFrameBufferFormat format = {};
        format.renderPass = renderPass;
        format.useDepthStencil = useDepthStencil;
        format.numOfAttachments = (uint32) attachmentDescriptions.size();

        return mFrameBufferFormats.move(format);
    }

    void VulkanRenderDevice::destroyFramebufferFormat(ID<FramebufferFormat> framebufferFormat) {
        auto &format = mFrameBufferFormats.get(framebufferFormat);
        vkDestroyRenderPass(mContext.device, format.renderPass, nullptr);

        mFrameBufferFormats.remove(framebufferFormat);
    }

    ID<Framebuffer> VulkanRenderDevice::createFramebuffer(const std::vector<ID<Texture>> &attachmentIds,
                                                           ID<FramebufferFormat> framebufferFormatId) {
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
        VkResult result = vkCreateFramebuffer(mContext.device, &framebufferInfo, nullptr, &framebuffer);
        VK_RESULT_ASSERT(result, "Filed to create framebuffer");

        VulkanFramebuffer vkFramebuffer = {};
        vkFramebuffer.framebuffer = framebuffer;
        vkFramebuffer.framebufferFormatId = framebufferFormatId;
        vkFramebuffer.width = width;
        vkFramebuffer.height = height;

        return mFrameBuffers.move(vkFramebuffer);
    }

    void VulkanRenderDevice::destroyFramebuffer(ID<Framebuffer> framebufferId) {
        VulkanFramebuffer fbo = mFrameBuffers.get(framebufferId);
        vkDestroyFramebuffer(mContext.device, fbo.framebuffer, nullptr);

        mFrameBuffers.remove(framebufferId);
    }

    ID<UniformSet> VulkanRenderDevice::createUniformSet(const UniformSetDesc &setDesc, ID<UniformLayout> uniformLayout) {
        auto &layout = mUniformLayouts.get(uniformLayout);

        const auto &uniformBuffers = setDesc.buffers;
        const auto &uniformTextures = setDesc.textures;
        const auto &properties = layout.properties;
        uint32 buffersCount = uniformBuffers.size();
        uint32 texturesCount = uniformTextures.size();

        if (buffersCount != properties.uniformBuffersCount || texturesCount != properties.samplersCount) {
            throw VulkanException("Incompatible uniform layout and uniform set descriptor");
        }

        if (properties.uniformBuffersCount == 0 && properties.samplersCount == 0) {
            throw VulkanException("Uniform layout has not textures and buffers to be bounded");
        }

        VkDescriptorSet descriptorSet = layout.allocator.allocateSet();

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

        vkUpdateDescriptorSets(mContext.device, (uint32) writeDescSets.size(), writeDescSets.data(), 0, nullptr);

        VulkanUniformSet uniformSet = {};
        uniformSet.uniformLayout = uniformLayout;
        uniformSet.descriptorSet = descriptorSet;

        return mUniformSets.move(uniformSet);
    }

    void VulkanRenderDevice::destroyUniformSet(ID<UniformSet> setId) {
        auto &uniformSet = mUniformSets.get(setId);
        auto &layout = mUniformLayouts.get(uniformSet.uniformLayout);

        layout.allocator.freeSet(uniformSet.descriptorSet);
        mUniformSets.remove(setId);
    }

    ID<UniformLayout> VulkanRenderDevice::createUniformLayout(const IRenderDevice::UniformLayoutDesc &layoutDesc) {
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

        result = vkCreateDescriptorSetLayout(mContext.device, &createInfo, nullptr, &descriptorSetLayout);
        VK_RESULT_ASSERT(result, "Failed to create descriptor set layout");

        VulkanUniformLayout uniformLayout;
        uniformLayout.properties.layout = descriptorSetLayout;
        uniformLayout.properties.samplersCount = texturesCount;
        uniformLayout.properties.uniformBuffersCount = buffersCount;
        uniformLayout.allocator.setProperties(uniformLayout.properties);

        return mUniformLayouts.move(uniformLayout);
    }

    void VulkanRenderDevice::destroyUniformLayout(ID<UniformLayout> layout) {
        auto &uniformLayout = mUniformLayouts.get(layout);
        auto &properties = uniformLayout.properties;

        vkDestroyDescriptorSetLayout(mContext.device, properties.layout, nullptr);
        mUniformLayouts.remove(layout);
    }

    ID<UniformBuffer> VulkanRenderDevice::createUniformBuffer(BufferUsage usage, uint32 size, const void *data) {
        VulkanUniformBuffer uniformBuffer = {};
        uniformBuffer.usage = usage;
        uniformBuffer.size = size;

        if (usage == BufferUsage::Static) {
            VulkanUtils::createBufferLocal(data, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           uniformBuffer.buffer, uniformBuffer.allocation);
        } else if (usage == BufferUsage::Dynamic) {
            VkMemoryPropertyFlags memoryPropertyFlags =
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            VulkanUtils::createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                      memoryPropertyFlags, uniformBuffer.buffer, uniformBuffer.allocation);
            VulkanUtils::updateBufferMemory(uniformBuffer.allocation, 0, size, data);
        } else {
            throw VulkanException("Undefined uniform buffer usage");
        }

        return mUniformBuffers.move(uniformBuffer);
    }

    void VulkanRenderDevice::updateUniformBuffer(ID<UniformBuffer> buffer, uint32 size, uint32 offset, const void *data) {
        const VulkanUniformBuffer &uniformBuffer = mUniformBuffers.get(buffer);

        if (uniformBuffer.usage != BufferUsage::Dynamic) {
            throw VulkanException("Attempt to update static uniform buffer");
        }

        if (offset + size > uniformBuffer.size) {
            throw VulkanException("Attempt to update out-of-buffer memory region for uniform buffer");
        }

        VulkanUtils::updateBufferMemory(uniformBuffer.allocation, offset, size, data);
    }

    void VulkanRenderDevice::destroyUniformBuffer(ID<UniformBuffer> bufferId) {
        VulkanUniformBuffer &uniformBuffer = mUniformBuffers.get(bufferId);
        VulkanUtils::destroyBuffer(uniformBuffer.buffer, uniformBuffer.allocation);

        mUniformBuffers.remove(bufferId);
    }

    ID<ShaderProgram> VulkanRenderDevice::createShaderProgram(const ProgramDesc &programDesc) {
        VulkanShaderProgram program = {};
        program.shaders.reserve(programDesc.shaders.size());

        VK_TRUE_ASSERT(programDesc.language == ShaderLanguage::SPIRV, "Compiling shaders from not SPIR-V languages is not supported");

        for (const auto &desc: programDesc.shaders) {
            VkShaderModule module;

            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.pCode = (const uint32 *) desc.source.data();
            createInfo.codeSize = (uint32) desc.source.size();

            auto result = vkCreateShaderModule(mContext.device, &createInfo, nullptr, &module);
            VK_RESULT_ASSERT(result, "Failed to create shader module");

            VulkanShader shader = {};
            shader.module = module;
            shader.shaderStage = VulkanDefinitions::shaderStageBit(desc.type);

            program.shaders.push_back(shader);
        }

        return mShaderPrograms.move(program);
    }

    void VulkanRenderDevice::destroyShaderProgram(ID<ShaderProgram> program) {
        auto &vulkanProgram = mShaderPrograms.get(program);

        for (auto &shader: vulkanProgram.shaders) {
            vkDestroyShaderModule(mContext.device, shader.module, nullptr);
        }

        mShaderPrograms.remove(program);
    }

    ID<GraphicsPipeline> VulkanRenderDevice::createGraphicsPipeline(PrimitiveTopology topology,
            ID<ShaderProgram> program,
            ID<VertexLayout> vertexLayout,
            ID<UniformLayout> uniformLayout,
            ID<FramebufferFormat> framebufferFormat,
            const IRenderDevice::PipelineRasterizationDesc &rasterizationDesc,
            const IRenderDevice::PipelineBlendStateDesc &blendStateDesc,
            const IRenderDevice::PipelineDepthStencilStateDesc &depthStencilStateDesc) {
        const auto &vkProgram = mShaderPrograms.get(program);
        const auto &vkUniformLayout = mUniformLayouts.get(uniformLayout);
        const auto &vkVertexLayout = mVertexLayouts.get(vertexLayout);
        const auto &vkFramebufferFormat = mFrameBufferFormats.get(framebufferFormat);

        auto framebufferColorAttachmentsCount =
                vkFramebufferFormat.useDepthStencil ?
                vkFramebufferFormat.numOfAttachments - 1 :
                vkFramebufferFormat.numOfAttachments;

        if (blendStateDesc.attachments.size() != framebufferColorAttachmentsCount) {
            throw VulkanException("Incompatible number of color and blend attachments for specified framebuffer format and blend state");
        }

        if (depthStencilStateDesc.depthTestEnable && !vkFramebufferFormat.useDepthStencil) {
            throw VulkanException("Specified framebuffer format does not support depth/stencil buffer usage");
        }

        VkResult result;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        VulkanUtils::createPipelineLayout(vkUniformLayout, pipelineLayout);

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
        VulkanUtils::createColorBlendState(blendStateDesc, (uint32) attachments.size(), attachments.data(), colorBlending);

        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        if (depthStencilStateDesc.depthTestEnable || depthStencilStateDesc.stencilTestEnable) {
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

        result = vkCreateGraphicsPipelines(mContext.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create graphics pipeline");
        }

        VulkanGraphicsPipeline graphicsPipeline;
        graphicsPipeline.pipeline = pipeline;
        graphicsPipeline.pipelineLayout = pipelineLayout;

        return mGraphicsPipelines.move(graphicsPipeline);
    }

    ID<GraphicsPipeline> VulkanRenderDevice::createGraphicsPipeline(
            ID<Surface> surface,
            PrimitiveTopology topology,
            ID<ShaderProgram> program,
            ID<VertexLayout> vertexLayout,
            ID<UniformLayout> uniformLayout,
            const PipelineRasterizationDesc &rasterizationDesc,
            const PipelineSurfaceBlendStateDesc &blendStateDesc,
            const PipelineDepthStencilStateDesc &depthStencilStateDesc) {
        const auto &vkProgram = mShaderPrograms.get(program);
        const auto &vkUniformLayout = mUniformLayouts.get(uniformLayout);
        const auto &vkVertexLayout = mVertexLayouts.get(vertexLayout);
        auto &vkSurface = mSurfaces.get(surface);
        auto &vkFramebufferFormat = vkSurface.swapChain.framebufferFormat;

        VkResult result;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        VulkanUtils::createPipelineLayout(vkUniformLayout, pipelineLayout);

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

        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        if (depthStencilStateDesc.depthTestEnable || depthStencilStateDesc.stencilTestEnable) {
            VulkanUtils::createDepthStencilState(depthStencilStateDesc, depthStencilState);
        } else {
            depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilState.depthTestEnable = VK_FALSE;
            depthStencilState.depthWriteEnable = VK_FALSE;
            depthStencilState.depthBoundsTestEnable = VK_FALSE;
            depthStencilState.stencilTestEnable = VK_FALSE;
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
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = vkFramebufferFormat.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        result = vkCreateGraphicsPipelines(mContext.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create graphics pipeline");
        }

        VulkanGraphicsPipeline graphicsPipeline;
        graphicsPipeline.pipeline = pipeline;
        graphicsPipeline.pipelineLayout = pipelineLayout;

        return mGraphicsPipelines.move(graphicsPipeline);
    }

    void VulkanRenderDevice::destroyGraphicsPipeline(ID<GraphicsPipeline> pipeline) {
        auto &vulkanPipeline = mGraphicsPipelines.get(pipeline);

        vkDestroyPipeline(mContext.device, vulkanPipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(mContext.device, vulkanPipeline.pipelineLayout, nullptr);

        mGraphicsPipelines.remove(pipeline);
    }

    void VulkanRenderDevice::drawListBegin() {
        mDrawListState = {};
        mDrawListState.commandBuffer = VulkanUtils::beginTmpCommandBuffer(mContext.graphicsTmpCommandPool);

        vkCmdSetLineWidth(mDrawListState.commandBuffer, 1);
    }

    void VulkanRenderDevice::drawListEnd() {
        VkCommandBuffer commandBuffer = mDrawListState.commandBuffer;
        vkCmdEndRenderPass(commandBuffer);
        vkEndCommandBuffer(commandBuffer);
        mDrawQueue.push_back(commandBuffer);
    }

    void VulkanRenderDevice::drawListBindSurface(
            ID<Surface> surfaceId,
            const IRenderDevice::Color &color,
            const IRenderDevice::Region &area) {
        // End previous render pass, if exists
        if (mDrawListState.frameBufferAttached) {
            vkCmdEndRenderPass(mDrawListState.commandBuffer);
        }

        // Reset state
        mDrawListState.resetFlags();

        VulkanSurface &surface = mSurfaces.get(surfaceId);
        const float clearDepth = 1.0f;
        const uint32 clearStencil = 0;

        VkCommandBuffer cmd = mDrawListState.commandBuffer;
        VkFramebuffer framebuffer = surface.swapChain.framebuffers[surface.currentImageIndex];

        uint32 viewportOffsetX = area.xOffset;
        uint32 viewportOffsetY = area.yOffset;
        uint32 viewportWidth = area.extent.x;
        uint32 viewportHeight = area.extent.y;

        VkClearValue clearValues[2];
        clearValues[0].color.float32[0] = color.components[0];
        clearValues[0].color.float32[1] = color.components[1];
        clearValues[0].color.float32[2] = color.components[2];
        clearValues[0].color.float32[3] = color.components[3];
        clearValues[1].depthStencil.depth = clearDepth;
        clearValues[1].depthStencil.stencil = clearStencil;

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = surface.swapChain.framebufferFormat.renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;

        renderPassBeginInfo.renderArea.extent.width = surface.width;
        renderPassBeginInfo.renderArea.extent.height = surface.height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.framebuffer = framebuffer;

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

        mDrawListState.frameBufferAttached = true;
    }

    void VulkanRenderDevice::drawListBindFramebuffer(
            ID<Framebuffer> framebufferId,
            const std::vector<Color> &colors,
            float32 clearDepth, uint32 clearStencil,
            const IRenderDevice::Region &area) {
        // End previous render pass, if exists
        if (mDrawListState.frameBufferAttached) {
            vkCmdEndRenderPass(mDrawListState.commandBuffer);
        }

        // Reset state
        mDrawListState.resetFlags();

        VulkanFramebuffer &fbo = mFrameBuffers.get(framebufferId);
        VulkanFrameBufferFormat &fboFormat = mFrameBufferFormats.get(fbo.framebufferFormatId);
        VkCommandBuffer cmd = mDrawListState.commandBuffer;

        uint32 viewportOffsetX = area.xOffset;
        uint32 viewportOffsetY = area.yOffset;
        uint32 viewportWidth = area.extent.x;
        uint32 viewportHeight = area.extent.y;

        uint32 colorValuesCount = (uint32) colors.size();
        uint32 clearValuesCount = colorValuesCount + 1;
        mClearValues.clear();
        mClearValues.reserve(colorValuesCount);

        for (size_t i = 0; i < colorValuesCount; i++) {
            VkClearValue colorClearValue = {};
            colorClearValue.color.float32[0] = colors[i].components[0];
            colorClearValue.color.float32[1] = colors[i].components[1];
            colorClearValue.color.float32[2] = colors[i].components[2];
            colorClearValue.color.float32[3] = colors[i].components[3];

            mClearValues.push_back(colorClearValue);
        }

        VkClearValue depthStencilClearValues = {};
        depthStencilClearValues.depthStencil = {clearDepth, clearStencil};
        mClearValues.push_back(depthStencilClearValues);

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = fboFormat.renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;

        renderPassBeginInfo.renderArea.extent.width = fbo.width;
        renderPassBeginInfo.renderArea.extent.height = fbo.height;
        renderPassBeginInfo.clearValueCount = clearValuesCount;
        renderPassBeginInfo.pClearValues = mClearValues.data();
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

        mDrawListState.frameBufferAttached = true;
    }

    void VulkanRenderDevice::drawListBindFramebuffer(
            ID<Framebuffer> framebufferId,
            const std::vector<Color> &colors,
            const IRenderDevice::Region &area) {
        drawListBindFramebuffer(framebufferId, colors, 1.0f, 0, area);
    }

    void VulkanRenderDevice::drawListBindPipeline(ID<GraphicsPipeline> graphicsPipelineId) {
        VK_TRUE_ASSERT(mDrawListState.frameBufferAttached, "No framebuffer attached");
        const auto &graphicsPipeline = mGraphicsPipelines.get(graphicsPipelineId);
        vkCmdBindPipeline(mDrawListState.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipeline);
        mDrawListState.pipelineLayout = graphicsPipeline.pipelineLayout;
        mDrawListState.pipelineAttached = true;
    }

    void VulkanRenderDevice::drawListBindUniformSet(ID<UniformSet> uniformSetId) {
        VK_TRUE_ASSERT(mDrawListState.pipelineAttached, "No pipeline attached");
        const auto &uniformSet = mUniformSets.get(uniformSetId);
        vkCmdBindDescriptorSets(mDrawListState.commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                mDrawListState.pipelineLayout,
                                0, 1,
                                &uniformSet.descriptorSet,
                                0, nullptr);
    }

    void VulkanRenderDevice::drawListBindIndexBuffer(ID<IndexBuffer> indexBufferId, IndicesType indicesType, uint32 offset) {
        VK_TRUE_ASSERT(mDrawListState.frameBufferAttached, "No pipeline attached");
        const auto &indexBuffer = mIndexBuffers.get(indexBufferId);
        vkCmdBindIndexBuffer(mDrawListState.commandBuffer, indexBuffer.vkBuffer, offset, VulkanDefinitions::indexType(indicesType));
        mDrawListState.indexBufferAttached = true;
    }

    void VulkanRenderDevice::drawListBindVertexBuffer(ID<VertexBuffer> vertexBufferId, uint32 binding, uint32 offset) {
        VK_TRUE_ASSERT(mDrawListState.frameBufferAttached, "No pipeline attached");
        const auto &vertexBuffer = mVertexBuffers.get(vertexBufferId);
        VkDeviceSize offsets[1] = { offset };
        vkCmdBindVertexBuffers(mDrawListState.commandBuffer, binding, 1, &vertexBuffer.vkBuffer, offsets);
        mDrawListState.vertexBufferAttached = true;
    }

    void VulkanRenderDevice::drawListDraw(uint32 verticesCount, uint32 instancesCount) {
        VK_TRUE_ASSERT(mDrawListState.vertexBufferAttached, "Vertex buffer is not attached: nothing to draw");
        vkCmdDraw(mDrawListState.commandBuffer, verticesCount, instancesCount, 0, 0);
    }

    void VulkanRenderDevice::drawListDrawIndexed(uint32 indicesCount, uint32 instancesCount) {
        VK_TRUE_ASSERT(mDrawListState.vertexBufferAttached, "Vertex buffer is not attached: nothing to draw");
        VK_TRUE_ASSERT(mDrawListState.indexBufferAttached, "Index buffer is not attached: nothing to draw");
        vkCmdDrawIndexed(mDrawListState.commandBuffer, indicesCount, instancesCount, 0, 0, 0);
    }

    ID<Surface> VulkanRenderDevice::getSurface(const std::string &surfaceName) {
        for (auto i = mSurfaces.begin(); i != mSurfaces.end(); ++i) {
            auto &window = *i;
            if (window.name == surfaceName) {
                return i.getID();
            }
        }
        return ID<Surface>();
    }

    void VulkanRenderDevice::getSurfaceSize(ID<Surface> surface, uint32 &width, uint32 &height) {
        auto &window = mSurfaces.get(surface);
        width = window.width;
        height = window.height;
    }

    void VulkanRenderDevice::swapBuffers(ID<Surface> surfaceId) {
        VK_TRUE_ASSERT(mSyncQueue.empty(), "Device must be explicitly synchronized before swap buffers call");

        auto &surface = mSurfaces.get(surfaceId);
        auto &swapChain = surface.swapChain;
        auto imageIndex = surface.currentImageIndex;

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 0;
        presentInfo.pWaitSemaphores = nullptr;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain.swapChainKHR;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        auto result = vkQueuePresentKHR(surface.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // Gen new surface properties
            surface.resizeSurface();
        } else {
            VK_RESULT_ASSERT(result, "Failed to present image to the surface");
        }

        if (surface.canPresentImages) {
            surface.acquireNextImage();
        }
    }


    void VulkanRenderDevice::flush() {
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = (uint32) mDrawQueue.size();
        submitInfo.pCommandBuffers = mDrawQueue.data();
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        auto result = vkQueueSubmit(mContext.graphicsQueue, 1, &submitInfo, nullptr);
        VK_RESULT_ASSERT(result, "Failed to submit draw lists to graphics queue");

        mSyncQueue.insert(mSyncQueue.end(), mDrawQueue.begin(), mDrawQueue.end());
        mDrawQueue.clear();
    }

    void VulkanRenderDevice::synchronize() {
        vkQueueWaitIdle(mContext.graphicsQueue);

        for (auto buffer: mSyncQueue) {
            VulkanUtils::destroyTmpComandBuffer(buffer, mContext.graphicsTmpCommandPool);
        }

        mSyncQueue.clear();
    }


    const std::vector<DataFormat> &VulkanRenderDevice::getSupportedTextureFormats() const {
        return mSupportedTextureDataFormats;
    }

    const std::vector<ShaderLanguage> &VulkanRenderDevice::getSupportedShaderLanguages() {
        return mSupportedShaderLanguages;
    }

    const std::string &VulkanRenderDevice::getDeviceName() const {
        static std::string mDeviceName = "VulkanDevice";
        return mDeviceName;
    }

    IRenderDevice::Type VulkanRenderDevice::getDeviceType() const {
        return IRenderDevice::Type::VulkanDevice;
    }

} // namespace ignimbrite