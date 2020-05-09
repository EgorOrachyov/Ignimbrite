#version 450

layout (location = 0) in vec3 inPosition;

layout (binding = 0) uniform ShadowParams 
{
	mat4 depthMVP;
} shadowParams;

void main()
{
	gl_Position = shadowParams.depthMVP * vec4(inPosition, 1.0);
}