#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform CommonParams
{
	mat4 viewProj;
	mat4 model;
	mat4 lightSpace;
	vec3 lightDir;
	vec3 cameraPos;
} commonParams;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outCameraPos;
layout (location = 2) out vec3 outLightVec;
layout (location = 3) out vec4 outShadowCoord;
layout (location = 4) out vec2 outTexCoords;
layout (location = 5) out vec4 outPosition;

const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main()
{
	gl_Position = commonParams.viewProj * commonParams.model * vec4(inPos, 1.0);

	vec4 pos = commonParams.model * vec4(inPos, 1.0);
	outNormal = mat3(commonParams.model) * inNormal;
	outLightVec = -normalize(commonParams.lightDir);
	outCameraPos = commonParams.cameraPos;
	outTexCoords = inUV;
	outPosition = pos;

	outShadowCoord = (biasMat * commonParams.lightSpace * commonParams.model) * vec4(inPos, 1.0);
}

