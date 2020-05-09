#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;

layout (std140, binding = 0) uniform MdParams 
{
	mat4 mvp;
} mdParams;

void main() {
	vec3 lightDir = normalize(vec3(0, 1, 1));
	float light = max(0.3, dot(inNormal, lightDir));

	outColor = vec4(vec3(light), 1.0f);
	outTexCoord = inTexCoord;
	gl_Position = mdParams.mvp * vec4(inPosition, 1.0f);
}