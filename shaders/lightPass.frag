#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gDiffuse;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gMR;
layout(binding = 3) uniform sampler2D gDepth;

layout(set= 1, binding = 0) uniform UniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
} ubo;

vec3 gLightDirection = normalize(vec3(0.577, -0.577, 0.577));
const vec3 LIGHT_COLOR = vec3(7.0);  // Changed to color vector

const float PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0) 
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float NormalDistributionGGX(vec3 n, vec3 h, float roughness) 
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(n, h), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return a2 / max(denom, 0.0000001);  // Avoid division by zero
}

float GeometrySchlickGGX(float NdotV, float roughness) 
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;  // Direct lighting remapping
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 n, vec3 v, vec3 l, float roughness) 
{
    float NdotV = max(dot(n, v), 0.0);
    float NdotL = max(dot(n, l), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 GetWorldPositionFromDepth(float depth, ivec2 fragCoords, mat4 invProj, mat4 invView, ivec2 resolution) 
{
    vec2 ndc;
    ndc.x = (float(fragCoords.x) / float(resolution.x)) * 2.0 - 1.0;
    ndc.y = (float(fragCoords.y) / float(resolution.y)) * 2.0 - 1.0;

    vec4 clipPos = vec4(ndc, depth, 1.0);
    vec4 viewPos = invProj * clipPos;
    viewPos /= viewPos.w;
    vec4 worldPos = invView * viewPos;
    return worldPos.xyz;
}

void main() 
{
    // Reconstruct world position
    float depth = texelFetch(gDepth, ivec2(gl_FragCoord.xy), 0).r;
    mat4 invProj = inverse(ubo.proj);
    mat4 invView = inverse(ubo.view);
    ivec2 res = textureSize(gDepth, 0);
    vec3 worldPos = GetWorldPositionFromDepth(depth, ivec2(gl_FragCoord.xy), invProj, invView, res);
    
    // Load G-buffer data
    vec4 albedo = texelFetch(gDiffuse, ivec2(gl_FragCoord.xy), 0);
    vec3 normal = texelFetch(gNormal, ivec2(gl_FragCoord.xy), 0).rgb * 2.0 - 1.0;
    vec2 mr = texelFetch(gMR, ivec2(gl_FragCoord.xy), 0).rg;
    float metallic = mr.r;
    float roughness = max(mr.g, 0.05);  // Clamp roughness to avoid artifacts

    // Calculate view vector (world space)
    vec3 V = normalize(ubo.cameraPosition - worldPos);
    vec3 L = normalize(-gLightDirection);  // Direction TO light source
    vec3 H = normalize(V + L);

    // Precompute dot products
    float NdotV = max(dot(normal, V), 0.0);
    float NdotL = max(dot(normal, L), 0.0);
    
    // Skip lighting if surface is backfaced
    if (NdotL <= 0.0) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Calculate Fresnel (F)
    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    
    // Calculate Normal Distribution (D)
    float D = NormalDistributionGGX(normal, H, roughness);
    
    // Calculate Geometry (G)
    float G = GeometrySmith(normal, V, L, roughness);
    
    // Combine specular BRDF
    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(NdotV, 0.0000001) * max(NdotL, 0.0000001);
    vec3 specular = numerator / max(denominator, 0.0000001);
    
    // Calculate diffuse (energy conservation)
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo.rgb / PI;
    
    // Final lighting calculation
    vec3 radiance = (diffuse + specular) * LIGHT_COLOR * NdotL;
    outColor = vec4(radiance, 1.0);
}