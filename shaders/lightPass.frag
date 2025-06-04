#version 450
layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gDiffuse;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gMR;   // metal+rough

void main() 
{
    vec4 albedo = texture(gDiffuse, fragTexCoord);
    vec3 normal_s = texture(gNormal, fragTexCoord).rgb * 2.0 - 1.0;
    vec3 N = normalize(normal_s);

    outColor = vec4(albedo);
}
