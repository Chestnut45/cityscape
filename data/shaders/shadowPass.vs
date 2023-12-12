#version 440

layout (location = 0) in vec3 vPos;

// Light space uniform block
layout(std140, binding = 5) uniform LightSpaceBlock
{
    mat4 viewProj;
};

void main()
{
    gl_Position = viewProj * vec4(vPos, 1.0);
}