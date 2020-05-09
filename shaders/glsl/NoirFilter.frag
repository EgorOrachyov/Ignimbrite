#version 450

layout (location = 0) in vec2 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D texScreen;

void main() {
    vec3 color = texture(texScreen, inTexCoords.xy).rgb;
    float grey = dot(color, vec3(0.3, 0.59, 0.11));
    outColor = vec4(vec3(grey), 1.0f);
}