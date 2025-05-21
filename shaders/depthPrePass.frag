#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    uint textureIndex;
} pc;

void main() 
{
//    uint idx = pc.textureIndex;
//    outColor = texture(nonuniformEXT(textures[idx]), fragTexCoord);
    outColor = vec4(1,1,1,1);
}