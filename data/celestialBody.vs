#version 440

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
    vec2 resolution;
};

uniform vec4 position;

// Vertex data
in vec3 vPos;

void main()
{
    // Scale vertex position by radius and center around camera
    gl_Position = viewProj * vec4(cameraPos.xyz + position.xyz + (vPos * position.w), 1.0);
}