#version 450

layout (location = 0) in vec2 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D texScreen;

// params for buffer linearization
layout (binding = 1) uniform DepthLinear 
{
    float near;
    float far;
} depthLinear;

float linearizeDepth(float d)
{
    float near = depthLinear.near;
    float far = depthLinear.far;

    return near * far / (far + d * (near - far));
}

void main() {
    float ld = linearizeDepth(texture(texScreen, inTexCoords.xy).r);
    outColor = vec4(vec3(ld), 1.0f);
}