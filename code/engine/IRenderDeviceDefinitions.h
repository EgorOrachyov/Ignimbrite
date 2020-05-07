/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_IRENDERDEVICEDEFINITIONS_H
#define IGNIMBRITE_IRENDERDEVICEDEFINITIONS_H

#include <Types.h>
#define BIT_SHIFT(x) (1u << x)

namespace ignimbrite {

    /** Shader Program stages */
    enum class ShaderType {
        Vertex,
        Fragment
    };

    /** Shader stage flag bits. Used for uniform  */
    enum class ShaderStageFlagBits {
        VertexBit = BIT_SHIFT(0u),
        FragmentBit = BIT_SHIFT(1u)
    };
    typedef uint32 ShaderStageFlags;

    /** Source code languages */
    enum class ShaderLanguage {
        HLSL,
        GLSL,
        SPIRV
    };

    /** All the possible data formats for textures and shader attributes */
    enum class DataFormat {
        R8G8B8_UNORM,
        R8G8B8A8_UNORM,

        R32_SFLOAT,
        R32G32_SFLOAT,
        R32G32B32_SFLOAT,
        R32G32B32A32_SFLOAT,

        D24_UNORM_S8_UINT,
        D32_SFLOAT_S8_UINT,
    };

    /** Buffer usage for frequently updated or static buffer (has performance impact) */
    enum class BufferUsage {
        Static,
        Dynamic
    };

    /** Data usage in vertex buffer among several vertex/instance draws */
    enum class VertexUsage {
        /** Data of that vertex buffer is iterated per vertex draw call */
        PerVertex,
        /** Data of that vertex buffer is iterated per instance draw call */
        PerInstance
    };

    /** Type indices for index buffer */
    enum class IndicesType {
        Uint32,
        Uint16
    };

    /** Specifies how to attach texture to framebuffer */
    enum class AttachmentType {
        Color,
        DepthStencil
    };

    /** Types of the supported textures (1D,2D,3D) */
    enum class TextureType {
        Texture2D,
        Cubemap
    };

    /** Texture usage for optimal target device representation */
    enum class TextureUsageBit {
        /** Could be used as framebuffer color attachment */
        ColorAttachment = BIT_SHIFT(0u),
        /** Could be used as framebuffer depth-stencil attachment */
        DepthStencilAttachment = BIT_SHIFT(1u),
        /** Could be used sampled from shader program as uniform texture */
        ShaderSampling = BIT_SHIFT(2u),
        /** Could be used as depth attachment and sampled from shader (with flag ShaderSampling) */
        DepthAttachment = BIT_SHIFT(3u) | DepthStencilAttachment,
    };
    typedef uint32 TextureUsageFlags;

    /** Num of samples **/
    enum class TextureSamples {
        Samples1
    };

    /** Image filtering */
    enum class SamplerFilter {
        Nearest,
        Linear
    };

    /** Image behavior on edges */
    enum class SamplerRepeatMode {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder
    };

    /** Vulkan has only predefined colors */
    enum class SamplerBorderColor {
        Black,
        White
    };

    enum class PolygonMode {
        Fill,
        Line,
        Point
    };

    enum class PolygonCullMode {
        Disabled,
        Front,
        Back
    };

    enum class PolygonFrontFace {
        FrontClockwise,
        FrontCounterClockwise
    };

    enum class CompareOperation {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    enum class StencilOperation {
        Keep,
        Zero,
        Replace,
        IncrementAndClamp,
        DecrementAndClamp,
        Invert,
        IncrementAndWrap,
        DecrementAndWrap
    };


    enum class LogicOperation {
        Clear,
        And,
        AndReverse,
        Copy,
        AndInverted,
        NoOp,
        Xor,
        Or,
        Nor,
        Equivalent,
        Invert,
        OrReverse,
        CopyInverted,
        OrInverted,
        Nand,
        Set
    };

    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate,
        /** For dual source blending modes */
        Src1Color,
        OneMinusSrc1Color,
        Src1Alpha,
        OneMinusSrc1Alpha
    };

    enum class BlendOperation {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum class PrimitiveTopology {
        PointList,
        LineList,
        LineStrip,
        TriangleStrip,
        TriangleFan,
        TriangleList
    };

} // namespace ignimbrite

#undef BIT_SHIFT

#endif //IGNIMBRITE_IRENDERDEVICEDEFINITIONS_H
