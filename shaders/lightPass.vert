#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

vec2 positions[3] = vec2[](
    vec2( 3.0, -1.0),
    vec2(-1.0, -1.0),
    vec2(-1.0,  3.0)
);

layout(location = 0) out vec2 fragTexCoord;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragTexCoord = (gl_Position.xy + 1.0) * 0.5;
}