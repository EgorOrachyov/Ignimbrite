#version 450

layout(location = 0) in vec2 inScreenCoords;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texScreen;

void main() {
	outColor = vec4(texture(texScreen, inScreenCoords).rgb, 1.0f);
}