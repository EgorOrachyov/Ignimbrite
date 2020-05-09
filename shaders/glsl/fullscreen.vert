#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec2 outScreenCoords;

void main() {
    outScreenCoords = inPosition.xy;
    gl_Position = vec4(inPosition, 1.0f);
}