#version 440

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
    vec2 resolution;
};

// Direct access to snow vertex buffer
layout(std140, binding = 1) buffer snowVerts
{
    vec4 vertPositions[];
};

uniform float delta;

in vec4 vPos;

void main()
{
    // Keep all the snow centered around the camera
    gl_Position = viewProj * vec4(vPos.xyz + cameraPos.xyz, 1.0);
    gl_PointSize = vPos.w;

    // Update the position after rendering
    vec4 nextPos = vPos;
    nextPos.y -= 0.1 * delta;
    nextPos.y = nextPos.y < -1.0 ? 1.0 : nextPos.y;
    vertPositions[gl_VertexID] = nextPos;
}