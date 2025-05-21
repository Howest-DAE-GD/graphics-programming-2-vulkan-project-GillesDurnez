#version 450
#extension GL_EXT_nonuniform_qualifier : require
//#extension GL_GOOGLE_include_directive : require

layout(set = 1, binding = 1) uniform sampler2D textures[];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    uint textureIndex;
} pc;

void main() 
{
    uint idx = pc.textureIndex;
    outColor = texture(nonuniformEXT(textures[idx]), fragTexCoord);
}