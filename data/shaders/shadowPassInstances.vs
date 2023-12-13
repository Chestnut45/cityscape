#version 440

// Light space uniform block
layout(std140, binding = 5) uniform LightSpaceBlock
{
    mat4 viewProj;
};

layout(std140, binding = 1) buffer InstanceBlock
{
    vec4 instancePositions[];
};

// Vertex data
layout (location = 0) in vec3 vPos;

void main()
{
    gl_Position = viewProj * vec4(vPos + instancePositions[gl_InstanceID].xyz, 1.0);
}