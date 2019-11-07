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
};

#endif //RENDERINGLIBRARY_VULKANDEFINITIONS_H
