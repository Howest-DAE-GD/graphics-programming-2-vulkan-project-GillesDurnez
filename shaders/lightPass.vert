#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

vec2 positions[3] = vec2[](
    vec2( 3.0, -1.0),
    vec2(-1.0, -1.0),
    vec2(-1.0,  3.0)
);

//layout(location = 0) out vec4 fragPosition;
//layout(location = 1) out vec3 fragNormal;
//layout(location = 2) out vec3 fragTangent;
//layout(location = 3) out vec2 fragTexCoord;

layout(set= 1, binding = 0) uniform UniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

//    mat4x4 worldViewProj = ubo.model * ubo.view * ubo.proj;
//
//    vec4 transformedPos = vec4(inPosition, 1.f) * worldViewProj;
//
//    
//    fragPosition = transformedPos;
//    fragNormal =  normalize(inNormal);
//    fragTangent = normalize(inTangent);
//    fragTexCoord = (gl_Position.xy + 1.0) * 0.5;
}