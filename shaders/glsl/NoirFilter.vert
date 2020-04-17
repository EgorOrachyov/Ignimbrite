#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec2 vTexCoords;
layout (location = 0) out vec2 fTexCoords;

void main() {
    fTexCoords = vTexCoords;
	gl_Position = vec4(vPosition.xy, 0.0f, 1.0f);
}