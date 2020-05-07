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
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outViewVec;
layout (location = 2) out vec3 outLightVec;
layout (location = 3) out vec4 outShadowCoord;
layout (location = 4) out vec2 outTexCoord;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{
	gl_Position = ubo.viewProj * ubo.model * vec4(inPos, 1.0);

	vec4 pos = ubo.model * vec4(inPos, 1.0);
	outNormal = mat3(ubo.model) * inNormal;
	outLightVec = -normalize(ubo.lightDir);
	outViewVec = -pos.xyz;			
	outTexCoord = inTexCoord;

	outShadowCoord = (biasMat * ubo.lightSpace * ubo.model) * vec4(inPos, 1.0);	
}

