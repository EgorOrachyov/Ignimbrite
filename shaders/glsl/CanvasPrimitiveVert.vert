#version 450

layout (location = 0) in vec4 inPosScale;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform UBO
{
	mat4 vp;
} ubo;


void main() 
{
	outColor = inColor;
	gl_Position = ubo.vp * vec4(inPosScale.xyz, 1.0);
	gl_PointSize = inPosScale.w;
}