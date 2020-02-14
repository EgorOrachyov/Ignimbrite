#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform bufferVals
{
	mat4 mvp;
} myBufferVals;

layout (location = 0) in vec4 vtPosition;
layout (location = 1) in vec4 vtColor;
layout (location = 2) in vec3 vtNormal;
layout (location = 3) in vec2 vtTexCoord;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;

void main() 
{
	vec3 lightDir = normalize(vec3(-1, -1, 0));
	float light = max(0.0, dot(vtNormal, lightDir)) + 0.3;

	outColor = vtColor * light;
	outTexCoord = vtTexCoord;
	gl_Position = myBufferVals.mvp * vtPosition;
}