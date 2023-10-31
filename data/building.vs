#version 150

// Camera uniform block
layout(std140) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
};

// Vertex data inputs
in vec3 vPos;
in vec3 vNorm;
in vec2 vUv;

out vec3 normal;
out vec2 texCoords;

void main()
{
    gl_Position = viewProj * vec4(vPos, 1);
    normal = vNorm;
    texCoords = vUv;
}