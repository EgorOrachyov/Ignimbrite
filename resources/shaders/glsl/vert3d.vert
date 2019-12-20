#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;

layout (binding = 0) uniform UBO 
{
	mat4 mvp;
	mat4 model;
	vec3 lightDir;
	vec3 ambient;
} ubo;


void main() 
{	
	vec3 normal = normalize(mat3(ubo.model) * inNormal);
	float light = max(0.0, dot(normal, normalize(ubo.lightDir)));

	outColor = inColor * light + vec4(ubo.ambient, 0.0);
	outTexCoord = inTexCoord;
	gl_Position = ubo.mvp * inPos;
}