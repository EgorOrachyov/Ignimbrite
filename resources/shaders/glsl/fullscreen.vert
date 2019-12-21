#version 450

layout (location = 0) in vec3 position;
layout (location = 0) out vec2 screenCoords;

void main() {
    screenCoords = position.xy;
    gl_Position = vec4(position, 1.0f);
}