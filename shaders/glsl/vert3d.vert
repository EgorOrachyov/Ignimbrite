#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform bufferVals {
	mat4 mvp;
} myBufferVals;

layout (location = 0) in vec3 vtPosition;
layout (location = 1) in vec3 vtNormal;
layout (location = 2) in vec2 vtTexCoord;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;

void main() {
	vec3 lightDir = normalize(vec3(0, 1, 1));
	float light = max(0.3, dot(vtNormal, lightDir));

	outColor = vec4(vec3(light), 1.0f);
	outTexCoord = vtTexCoord;
	gl_Position = myBufferVals.mvp * vec4(vtPosition, 1.0f);
}