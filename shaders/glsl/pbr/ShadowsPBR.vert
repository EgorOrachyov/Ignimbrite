#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

layout (binding = 0) uniform UBO
{
	mat4 viewProj;
	mat4 model;
	mat4 lightSpace;
	vec3 lightDir;
	vec3 cameraPosition;
} ubo;

layout (location = 0) out vec3 outViewVec;
layout (location = 1) out vec3 outLightVec;
layout (location = 2) out vec4 outShadowCoord;
layout (location = 3) out vec2 outTexCoord;
layout (location = 4) out vec4 outPosition;
layout (location = 5) out mat3 outTBN;

const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

void main()
{
    vec3 N    = normalize(mat3(ubo.model) * inNormal);;
    vec3 T    = normalize(mat3(ubo.model) * inTangent);
	vec3 B    = normalize(mat3(ubo.model) * inBitangent);
	outTBN    = mat3(T,B,N);

	outLightVec = -normalize(ubo.lightDir);
	outTexCoord = inTexCoord;

	vec4 pos       = ubo.model * vec4(inPos, 1.0);
	outViewVec     = ubo.cameraPosition - pos.xyz;
    outPosition    = pos;
	outShadowCoord = (biasMat * ubo.lightSpace * ubo.model) * vec4(inPos, 1.0);

	gl_Position = ubo.viewProj * ubo.model * pos;
}

