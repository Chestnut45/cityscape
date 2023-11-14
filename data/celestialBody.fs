#version 150

// Camera uniform block
layout(std140) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
    vec2 resolution;
};

uniform vec4 color;

out vec4 finalColor;

void main()
{
    finalColor = color;
}