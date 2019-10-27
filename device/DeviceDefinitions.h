//
// Created by Egor Orachyov on 2019-10-27.
//

#ifndef VULKANRENDERER_DEVICEDEFINITIONS_H
#define VULKANRENDERER_DEVICEDEFINITIONS_H

#include <Types.h>

/** Shader Program stages */
enum class ShaderType {
    Vertex,
    Fragment
};

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
};

/** Num of samples **/
enum TextureSamples {
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



#endif //VULKANRENDERER_DEVICEDEFINITIONS_H
