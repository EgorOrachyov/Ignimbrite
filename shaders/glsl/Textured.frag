#version 450

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D texAlbedo;

void main() {
   outColor = vec4(texture(texAlbedo, inTexCoord).rgb, 1.0f) * inColor;
}