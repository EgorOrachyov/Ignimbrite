#version 450

layout(location = 0) out vec4 outupColor;
layout(location = 0) in vec2 screenCoords;

layout(location = 0) uniform sampler2D texture0;

void main() {
	outupColor = vec4(texture(texture0, screenCoords).rgb, 1.0f);
}