#version 440

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    vec2 resolution;
};

// Position in world space and radius of the celestial body
uniform vec4 positionRadius;

// Vertex data
in vec3 vPos;

void main()
{
    // Scale vertex position by radius and center around camera
    gl_Position = viewProj * vec4(cameraPos.xyz + positionRadius.xyz + (vPos * positionRadius.w), 1.0);
}