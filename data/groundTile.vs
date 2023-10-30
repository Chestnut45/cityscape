#version 150

const uint MAX_INSTANCES = 128u;

// Camera uniform block
layout(std140) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
};

// Instance uniform block
layout(std140) uniform InstanceBlock
{
    vec4 instancePos[MAX_INSTANCES];
};

// Vertex data
in vec3 vPos;
in vec3 vNorm;
in vec2 vUV;

out vec3 fNorm;
out vec2 texCoords;

void main()
{
    gl_Position = viewProj * vec4(vPos + instancePos[gl_InstanceID].xyz, 1);
    fNorm = vNorm;
    texCoords = vUV;
}