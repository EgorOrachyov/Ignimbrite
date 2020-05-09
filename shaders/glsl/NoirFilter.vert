#version 450

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoords;

layout (location = 0) out vec2 outTexCoords;

void main() {
    outTexCoords = inTexCoords;
    gl_Position = vec4(inPosition.xy, 0.0f, 1.0f);
}