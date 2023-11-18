#version 440

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
    vec2 resolution;
};

// Vertex data inputs
in vec3 vPos;
in vec3 vNorm;
in vec2 vUv;

// Fragment outputs
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 texCoords;

void main()
{
    gl_Position = viewProj * vec4(vPos, 1);

    // Varying outputs
    fragPos = vPos;
    normal = vNorm;
    texCoords = vUv;
}