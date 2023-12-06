#version 440

const uint MAX_INSTANCES = 128u;

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    vec2 resolution;
};

// Instance uniform block
layout(std140, binding = 1) uniform InstanceBlock
{
    vec4 instancePos[MAX_INSTANCES];
};

// Vertex data
in vec3 vPos;
in vec3 vNorm;
in vec2 vUV;

// Fragment outputs
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 texCoords;

void main()
{
    vec4 pos = vec4(vPos + instancePos[gl_InstanceID].xyz, 1);
    gl_Position = viewProj * pos;

    // Varying outputs
    fragPos = pos.xyz;
    normal = vNorm;
    texCoords = vUV;
}