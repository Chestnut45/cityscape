#version 440

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
    vec2 resolution;
};

// Vertex data
in vec3 vPos;

// Output vertex data directly as texture coordinates.
// Since the skyboxes will always be centered around the camera,
// we can use the interpolated fragment coordinates as an
// unnormalized direction vector to sample the cube textures with.
out vec3 texCoords;

void main()
{
    texCoords = vPos;
    vec4 pos = viewProj * vec4(vPos + cameraPos.xyz, 1.0);
    
    // Ensure NDCs have max depth value
    gl_Position = pos.xyww;
}