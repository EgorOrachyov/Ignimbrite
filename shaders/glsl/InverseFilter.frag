#version 450

layout (location = 0) in vec2 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D texScreen;

void main() {
    vec3 color = texture(texScreen, inTexCoords.xy).rgb;
    outColor = vec4(vec3(1.0f) - color, 1.0f);
}