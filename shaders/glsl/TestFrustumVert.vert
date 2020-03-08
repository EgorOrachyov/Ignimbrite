#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform UBO {
	mat4 viewProj;
	mat4 model;
	vec4 color;
} ubo;

layout (location = 0) in vec4 vtPosition;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = ubo.color;
	gl_Position = ubo.viewProj * ubo.model * vtPosition;
}