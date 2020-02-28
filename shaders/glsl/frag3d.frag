#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 outColor;
layout (binding = 1) uniform sampler2D texSampler;

layout (location = 0) in vec4 vtColor;
layout (location = 1) in vec2 vtTexCoord;

void main() {
   outColor = vec4(texture(texSampler, vtTexCoord).rgb, 1.0f) * vtColor;
}