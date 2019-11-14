//
// Created by Egor Orachyov on 2019-11-03.
//

#ifndef RENDERINGLIBRARY_VULKANDEFINITIONS_H
#define RENDERINGLIBRARY_VULKANDEFINITIONS_H

#include "vulkan/vulkan.h"
#include "renderer/DeviceDefinitions.h"
#include "VulkanErrors.h"

class VulkanDefinitions {
public:

    static VkFormat dataFormat(DataFormat format) {
        switch (format) {
            case DataFormat::D24_UNORM_S8_UINT:
                return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;
            default:
                throw InvalidEnum();
        }
    }

    static VkVertexInputRate vertexInputRate(VertexUsage vertexUsage) {
        switch (vertexUsage) {
            case VertexUsage::PerVertex:
                return VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
            case VertexUsage::PerInstance:
                return VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;
            default:
                throw InvalidEnum();
        }
    }

    static VkImageViewType imageViewType(TextureType type) {
        switch (type) {
            case TextureType::Texture2D:
                return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
            default:
                throw InvalidEnum();
        }
    }

    static VkImageType imageType(TextureType type) {
        switch (type) {
            case TextureType::Texture2D:
                return VkImageType::VK_IMAGE_TYPE_2D;
            default:
                throw InvalidEnum();
        }
    }

    static VkBorderColor borderColor(SamplerBorderColor color) {
        switch (color) {
            case SamplerBorderColor::Black:
                return VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            default:
                throw InvalidEnum();
        }
    }

    static VkFilter filter(SamplerFilter filter) {
        switch (filter) {
            case SamplerFilter::Linear:
                return VkFilter::VK_FILTER_LINEAR;
            case SamplerFilter::Nearest:
                return VkFilter::VK_FILTER_NEAREST;
            default:
                throw InvalidEnum();
        }
    }

    static VkSamplerAddressMode samplerAddressMode(SamplerRepeatMode mode) {
        switch (mode) {
            case SamplerRepeatMode::ClampToBorder:
                return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            case SamplerRepeatMode::ClampToEdge:
                return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerRepeatMode::MirroredRepeat:
                return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case SamplerRepeatMode::Repeat:
                return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
            default:
                throw InvalidEnum();
        }
    }

    static VkSampleCountFlagBits samplesCount(TextureSamples samples) {
        switch (samples) {
            case TextureSamples::Samples1:
                return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
            default:
                throw InvalidEnum();
        }
    }

    static VkImageLayout imageLayout(AttachmentType type) {
        switch (type) {
            case AttachmentType::Color:
                return VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case AttachmentType::DepthStencil:
                return VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            default:
                throw InvalidEnum();
        }
    }

    static VkSamplerMipmapMode samplerMipmapMode(SamplerFilter mipmapMode) {
        switch (mipmapMode) {
            case SamplerFilter::Linear:
                return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
            case SamplerFilter::Nearest:
                return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
            default:
                throw InvalidEnum();
        }
    }

    static VkSampleCountFlagBits sampleCount(TextureSamples samples) {
        switch (samples) {
            case TextureSamples::Samples1:
                return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
            default:
                throw InvalidEnum();
        }
    }

    static VkImageUsageFlags imageUsageFlags(uint32 flags) {
        VkImageUsageFlags result = 0;

        if (flags & (uint32) TextureUsageBit::ShaderSampling) {
            result |= (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        if (flags & (uint32) TextureUsageBit::ColorAttachment) {
            result |= (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        if (flags & (uint32) TextureUsageBit::DepthStencilAttachment) {
            result |= (uint32) VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        return result;
    }

    static VkShaderStageFlags shaderStageFlags(ShaderStageFlags flags) {
        VkShaderStageFlags result = 0;

        if (flags & (uint32) ShaderStageFlagBits::VertexBit) {
            result |= (uint32) VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
        }

        if (flags & (uint32) ShaderStageFlagBits::FragmentBit) {
            result |= (uint32) VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        return result;
    }
};

#endif //RENDERINGLIBRARY_VULKANDEFINITIONS_H
