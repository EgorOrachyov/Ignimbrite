#version 450

layout (location = 0) in vec3 inPosition;

layout (binding = 0) uniform UBO 
{
	mat4 depthMVP;
} ubo;

void main()
{
	gl_Position = ubo.depthMVP * vec4(inPosition, 1.0);
}