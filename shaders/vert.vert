#version 450

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 mvp;
} ubo;

out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() 
{
	gl_Position = vec4(inPos.xyz, 1.0);
}