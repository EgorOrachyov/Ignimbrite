/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANDEFINITIONS_H
#define IGNIMBRITELIBRARY_VULKANDEFINITIONS_H

#include <vulkan/vulkan.h>
#include <ignimbrite/RenderDeviceDefinitions.h>
#include <VulkanErrors.h>

namespace ignimbrite {

    class VulkanDefinitions {
    public:

        static VkFormat dataFormat(DataFormat format) {
            switch (format) {
                case DataFormat::R8G8B8_UNORM:
                    return VkFormat::VK_FORMAT_R8G8B8_UNORM;
                case DataFormat::R8G8B8A8_UNORM:
                    return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
                case DataFormat::R32_SFLOAT:
                    return VkFormat::VK_FORMAT_R32_SFLOAT;
                case DataFormat::R32G32_SFLOAT:
                    return VkFormat::VK_FORMAT_R32G32_SFLOAT;
                case DataFormat::R32G32B32_SFLOAT:
                    return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
                case DataFormat::R32G32B32A32_SFLOAT:
                    return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
                case DataFormat::D24_UNORM_S8_UINT:
                    return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;
                case DataFormat::D32_SFLOAT_S8_UINT:
                    return VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;
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

        static VkShaderStageFlagBits shaderStageBit(ShaderType type) {
            switch (type) {
                case ShaderType::Vertex:
                    return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
                case ShaderType::Fragment:
                    return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
                default:
                    throw InvalidEnum();
            }
        }

        static VkPrimitiveTopology primitiveTopology(PrimitiveTopology topology) {
            switch (topology) {
                case PrimitiveTopology::TriangleList:
                    return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                case PrimitiveTopology::TriangleStrip:
                    return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                case PrimitiveTopology::TriangleFan:
                    return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
                case PrimitiveTopology::PointList:
                    return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                case PrimitiveTopology::LineList:
                    return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case PrimitiveTopology::LineStrip:
                    return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
                default:
                    throw InvalidEnum();
            }
        }

        static VkPolygonMode polygonMode(PolygonMode mode) {
            switch (mode) {
                case PolygonMode::Fill:
                    return VkPolygonMode::VK_POLYGON_MODE_FILL;
                case PolygonMode::Line:
                    return VkPolygonMode::VK_POLYGON_MODE_LINE;
                case PolygonMode::Point:
                    return VkPolygonMode::VK_POLYGON_MODE_POINT;
                default:
                    throw InvalidEnum();
            }
        }

        static VkCullModeFlagBits cullModeFlagBits(PolygonCullMode mode) {
            switch (mode) {
                case PolygonCullMode::Front:
                    return VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT;
                case PolygonCullMode::Back:
                    return VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
                case PolygonCullMode::Disabled:
                    return VkCullModeFlagBits::VK_CULL_MODE_NONE;
                default:
                    throw InvalidEnum();
            }
        }

        static VkFrontFace frontFace(PolygonFrontFace face) {
            switch (face) {
                case PolygonFrontFace::FrontClockwise:
                    return VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
                case PolygonFrontFace::FrontCounterClockwise:
                    return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
                default:
                    throw InvalidEnum();
            }
        }

        static VkColorComponentFlags colorComponentFlags(bool r, bool g, bool b, bool a) {
            VkColorComponentFlags result = 0x0;

            if (r) result |= VK_COLOR_COMPONENT_R_BIT;
            if (g) result |= VK_COLOR_COMPONENT_G_BIT;
            if (b) result |= VK_COLOR_COMPONENT_B_BIT;
            if (a) result |= VK_COLOR_COMPONENT_A_BIT;

            return result;
        }

        static VkBlendFactor blendFactor(BlendFactor factor) {
            switch (factor) {
                case BlendFactor::Zero:
                    return VkBlendFactor::VK_BLEND_FACTOR_ZERO;
                case BlendFactor::One:
                    return VkBlendFactor::VK_BLEND_FACTOR_ONE;
                default:
                    throw InvalidEnum();
            }
        }

        static VkBlendOp blendOperation(BlendOperation operation) {
            switch (operation) {
                case BlendOperation::Add:
                    return VkBlendOp::VK_BLEND_OP_ADD;
                case BlendOperation::Subtract:
                    return VkBlendOp::VK_BLEND_OP_SUBTRACT;
                case BlendOperation::ReverseSubtract:
                    return VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT;
                case BlendOperation::Min:
                    return VkBlendOp::VK_BLEND_OP_MIN;
                case BlendOperation::Max:
                    return VkBlendOp::VK_BLEND_OP_MAX;
                default:
                    throw InvalidEnum();
            }
        }

        static VkLogicOp logicOperation(LogicOperation operation) {
            switch (operation) {
                case LogicOperation::Clear:
                    return VkLogicOp::VK_LOGIC_OP_CLEAR;
                case LogicOperation::And:
                    return VkLogicOp::VK_LOGIC_OP_AND;
                case LogicOperation::AndReverse:
                    return VkLogicOp::VK_LOGIC_OP_AND_REVERSE;
                case LogicOperation::Copy:
                    return VkLogicOp::VK_LOGIC_OP_COPY;
                case LogicOperation::AndInverted:
                    return VkLogicOp::VK_LOGIC_OP_AND_INVERTED;
                case LogicOperation::NoOp:
                    return VkLogicOp::VK_LOGIC_OP_NO_OP;
                case LogicOperation::Xor:
                    return VkLogicOp::VK_LOGIC_OP_XOR;
                case LogicOperation::Or:
                    return VkLogicOp::VK_LOGIC_OP_OR;
                case LogicOperation::Nor:
                    return VkLogicOp::VK_LOGIC_OP_NOR;
                case LogicOperation::Equivalent:
                    return VkLogicOp::VK_LOGIC_OP_EQUIVALENT;
                case LogicOperation::Invert:
                    return VkLogicOp::VK_LOGIC_OP_AND_INVERTED;
                case LogicOperation::OrReverse:
                    return VkLogicOp::VK_LOGIC_OP_OR_REVERSE;
                case LogicOperation::CopyInverted:
                    return VkLogicOp::VK_LOGIC_OP_COPY_INVERTED;
                case LogicOperation::OrInverted:
                    return VkLogicOp::VK_LOGIC_OP_OR_INVERTED;
                case LogicOperation::Nand:
                    return VkLogicOp::VK_LOGIC_OP_NAND;
                case LogicOperation::Set:
                    return VkLogicOp::VK_LOGIC_OP_SET;
                default:
                    throw InvalidEnum();
            }
        }

        static VkCompareOp compareOperation(CompareOperation operation) {
            switch (operation) {
                case CompareOperation::Never:
                    return VkCompareOp::VK_COMPARE_OP_NEVER;
                case CompareOperation::Less:
                    return VkCompareOp::VK_COMPARE_OP_LESS;
                case CompareOperation::Equal:
                    return VkCompareOp::VK_COMPARE_OP_EQUAL;
                case CompareOperation::LessOrEqual:
                    return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
                case CompareOperation::Greater:
                    return VkCompareOp::VK_COMPARE_OP_GREATER;
                case CompareOperation::NotEqual:
                    return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
                case CompareOperation::GreaterOrEqual:
                    return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
                case CompareOperation::Always:
                    return VkCompareOp::VK_COMPARE_OP_ALWAYS;
                default:
                    throw InvalidEnum();
            }
        }

        static VkStencilOp stencilOperation(StencilOperation operation) {
            switch (operation) {
                case StencilOperation::Keep:
                    return VkStencilOp::VK_STENCIL_OP_KEEP;
                case StencilOperation::Zero:
                    return VkStencilOp::VK_STENCIL_OP_ZERO;
                case StencilOperation::Replace:
                    return VkStencilOp::VK_STENCIL_OP_REPLACE;
                case StencilOperation::IncrementAndClamp:
                    return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP;
                case StencilOperation::DecrementAndClamp:
                    return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP;
                case StencilOperation::Invert:
                    return VkStencilOp::VK_STENCIL_OP_INVERT;
                case StencilOperation::IncrementAndWrap:
                    return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP;
                case StencilOperation::DecrementAndWrap:
                    return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP;
                default:
                    throw InvalidEnum();
            }
        }

        static VkIndexType indexType(IndicesType type) {
            switch (type) {
                case IndicesType::Uint16:
                    return VkIndexType::VK_INDEX_TYPE_UINT16;
                case IndicesType::Uint32:
                    return VkIndexType::VK_INDEX_TYPE_UINT32;
                default:
                    throw InvalidEnum();
            }
        }

        static uint32 getFormatSize(DataFormat format) {
            // returns size in bytes
            switch (format) {
                case DataFormat::R8G8B8_UNORM:
                    return 3;
                case DataFormat::R8G8B8A8_UNORM:
                    return 4;
                case DataFormat::R32_SFLOAT:
                    return 4;
                case DataFormat::R32G32_SFLOAT:
                    return 8;
                case DataFormat::R32G32B32_SFLOAT:
                    return 12;
                case DataFormat::R32G32B32A32_SFLOAT:
                    return 16;
                case DataFormat::D24_UNORM_S8_UINT:
                    return 4;
                case DataFormat::D32_SFLOAT_S8_UINT:
                    return 8;
                default:
                    throw InvalidEnum();
            }
        }
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANDEFINITIONS_H
