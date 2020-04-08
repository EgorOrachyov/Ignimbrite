#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 fColor;
layout (location = 0) in vec2 fTexCoords;

layout (binding = 0) uniform sampler2D Texture0;

void main() {
    vec3 color = texture(Texture0, fTexCoords.xy).rgb;
    fColor = vec4(vec3(1.0f) - color, 1.0f);
}