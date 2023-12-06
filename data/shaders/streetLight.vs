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

// Instance SSBO
layout(std430, binding = 1) buffer InstanceSSBO
{
    vec4 instancePosition[];
};

// Vertex data inputs
in vec3 vPos;
in vec3 vColor;
in vec3 vNorm;
in vec2 vUv1;
in vec2 vUv2;

// Per-fragment outputs
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 color;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec2 texCoords1;
layout(location = 4) out vec2 texCoords2;

void main()
{
    gl_Position = viewProj * vec4(vPos + instancePosition[gl_InstanceID].xyz, 1);

    // Varying outputs
    fragPos = vPos + instancePosition[gl_InstanceID].xyz;
    color = vColor;
    normal = vNorm;
    texCoords1 = vUv1;
    texCoords2 = vUv2;
}