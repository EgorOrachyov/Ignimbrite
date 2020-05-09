#version 450

layout (binding = 1) uniform sampler2D texShadowMap;           // Linear
layout (binding = 2) uniform sampler2D texAlbedo;           // sRGB
layout (binding = 3) uniform sampler2D texAO;               // Linear
layout (binding = 4) uniform sampler2D texMetalRoughness;   // Linear
layout (binding = 5) uniform sampler2D texNormal;           // Linear
layout (binding = 6) uniform sampler2D texEmissive;         // Linear ?

layout (location = 0) in vec3 inViewVec;
layout (location = 1) in vec3 inLightVec;
layout (location = 2) in vec4 inShadowCoord;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) in vec4 inPosition;
layout (location = 5) in mat3 inTBN;

layout (location = 0) out vec4 outColor;

#define enablePCF 1
#define PI 3.14159265359

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

// Normal Distribution function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
   float a      = roughness * roughness;
   float a2     = a * a;
   float NdotH  = max(dot(N, H), 0.0);
   float NdotH2 = NdotH * NdotH;
   float nom    = a2;
   float denom  = NdotH2 * (a2 - 1.0f) + 1.0f;
         denom  = PI * denom * denom;
   return nom / denom;
}

// Geometry part
float GeometrySchlickGGX(float NdotV, float roughness)
{
   float r = (roughness + 1.0);
   float k = (r*r) / 8.0;
   float nom = NdotV;
   float denom = NdotV * (1.0 - k) + k;
   return nom / denom;
}

// Geometry function (with Smith optimizaion)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
   float NdotV = max(dot(N, V), 0.0);
   float NdotL = max(dot(N, L), 0.0);
   float ggx2 = GeometrySchlickGGX(NdotV, roughness);
   float ggx1 = GeometrySchlickGGX(NdotL, roughness);
   return ggx1 * ggx2;
}

// Reflectance by freshel equation (aproximation)
vec3 fresnelSchlick(float NdotL, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - NdotL, 5.0);
}

// Extract normal accordingly to tangent space
vec3 getNormal() {
    vec3 normal = texture(texNormal, inTexCoord).rgb;
    normal = normalize(normal * 2.0f - 1.0f);
    return normalize(inTBN * normal);
}

void main()
{
    float shadow = (enablePCF == 1) ?
        filterPCF(inShadowCoord / inShadowCoord.w) :
        textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

    vec3 emissive   = texture(texEmissive,inTexCoord).rgb;
    vec3 albedo     = pow(texture(texAlbedo,inTexCoord).rgb, vec3(2.2));      // remember, abledo in sRGB
    float metallic  = texture(texMetalRoughness,inTexCoord).b;
    float roughness = texture(texMetalRoughness,inTexCoord).g;
    float ao        = texture(texAO,inTexCoord).r;

    vec3 N = getNormal();               // World space normal vector
    vec3 V = normalize(inViewVec);      // From fragment to camera
    vec3 L = normalize(inLightVec);     // From fragment to the light
    vec3 H = normalize(L + V);          // Half-way (reflected ray)

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    float NdotL = max(dot(N,L), 0.0f);
    float NdotV = max(dot(N,V), 0.0f);
    vec3 lightColor = vec3(1.0f);
    vec3 radiance = lightColor;

    float NDF = DistributionGGX(N,H,roughness);
    float G = GeometrySmith(N,V,L,roughness);
    vec3  F = fresnelSchlick(NdotV, F0);

    vec3 Ks = F;                                      // Energy save law
    vec3 Kd = (vec3(1.0f) - Ks) * (1.0f - metallic);  // Metals has low diffuse lightnig effect

    vec3 nominator    = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular     = nominator / denominator;

    vec3 Lo = (Kd * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.04) * albedo * vec3(ao);
    vec3 color = ambient + Lo * vec3(shadow) + emissive;
    vec3 gammaCorrect = pow(color, vec3(1.0f / 2.2f));

    outColor = vec4(gammaCorrect, 1.0f);
}
