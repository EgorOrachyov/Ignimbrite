#version 450

layout (location = 0) in vec4 inPosition;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform FrustumParams 
{
	mat4 viewProj;
	mat4 model;
	vec4 color;
} frustumParams;

void main() {
	outColor = frustumParams.color;
	gl_Position = frustumParams.viewProj * frustumParams.model * inPosition;
}