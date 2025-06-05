#version 450
#extension GL_EXT_nonuniform_qualifier : require
//#extension GL_GOOGLE_include_directive : require

const uint UINT32_MAX = 0xFFFFFFFFu;

layout(set = 1, binding = 1) uniform sampler2D textures[];

layout(location = 0) in vec4 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec2 outMR;


layout(push_constant) uniform PushConstants {
    uint textureIndex;
    uint normalIndex;
    uint metalnessIndex;
    uint roughnessIndex;
} pc;

void main() 
{
    vec4 albedo;
    vec3 normal;
    float metalness;  
    float roughness;

    if (pc.textureIndex == UINT32_MAX)
    {
        albedo = vec4(0,0,0,1);
    }
    else
    {
        albedo = texture(textures[pc.textureIndex], fragTexCoord);
    }

    if (pc.normalIndex == UINT32_MAX)
    {
        normal = vec3(0,0,0);
    }
    else
    {
        normal = textureLod(textures[pc.normalIndex],   fragTexCoord, 1.f).xyz * 2.0 - 1.0;
    }

    if (pc.metalnessIndex == UINT32_MAX)
    {
        metalness = 0.0f;
    }
    else
    {
        metalness = texture(textures[pc.metalnessIndex],fragTexCoord).b;
    }

    if (pc.roughnessIndex == UINT32_MAX)
    {
        roughness = 0.0f;
    }
    else
    {
        roughness = texture(textures[pc.roughnessIndex],fragTexCoord).g;
    }

    vec3 T = normalize(fragTangent);
    vec3 B = normalize(cross(normalize(fragNormal), T));
    vec3 N = normalize(fragNormal);

    mat3 TBN = mat3(T, B, N);

    vec3 mN = TBN * normal;

    outAlbedo = albedo;
    outNormal = vec4(normalize(mN) * 0.5 + 0.5, 1.0);
    outMR     = vec2(metalness, roughness);
}