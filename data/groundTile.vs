#version 150

const uint MAX_INSTANCES = 128u;

// Camera uniform block
layout(std140) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
    vec2 resolution;
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

// Fragment outputs
out vec3 fragPos;
out vec3 normal;
out vec2 texCoords;

void main()
{
    vec4 pos = vec4(vPos + instancePos[gl_InstanceID].xyz, 1);
    gl_Position = viewProj * pos;

    // Varying outputs
    fragPos = pos.xyz;
    normal = vNorm;
    texCoords = vUV;
}