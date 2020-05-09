#version 450

layout (binding = 1) uniform sampler2D texShadowMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inCameraPos;
layout (location = 2) in vec3 inLightVec;
layout (location = 3) in vec4 inShadowCoord;
layout (location = 4) in vec2 inTexCoord;
layout (location = 5) in vec4 inPosition;

layout (location = 0) out vec4 outColor;

#define enablePCF 1
#define ambient 0.2

float textureProj(vec4 shadowCoord, vec2 offset)
{
	float bias = 0.01;

	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture(texShadowMap, shadowCoord.st + offset).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z - bias) 
		{
			shadow = 0.0;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(texShadowMap, 0);
	float dx = 1 / float(texDim.x);

	vec2 offset = vec2(mod(gl_FragCoord.x, 2), mod(gl_FragCoord.y, 2));
	offset.y = offset.x;
	
	if (offset.y > 1.1)
	{
		offset.y = 0;
	}

	float shadowFactor = (
		textureProj(sc, (offset + vec2(-1.5, 0.5)) * dx) +
		textureProj(sc, (offset + vec2(0.5, 0.5)) * dx) +
		textureProj(sc, (offset + vec2(-1.5, -1.5)) * dx) +
		textureProj(sc, (offset + vec2(0.5, -1.5)) * dx)
		) * 0.25;

	return shadowFactor;
}

void main() 
{	
	float shadow = (enablePCF == 1) ? 
		filterPCF(inShadowCoord / inShadowCoord.w) : 
		textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inPosition.xyz - inCameraPos);
	vec3 R = normalize(-reflect(L, N));
	vec3 c = vec3(max(dot(N, L) * shadow, ambient));

	outColor = vec4(c, 1.0);
}
