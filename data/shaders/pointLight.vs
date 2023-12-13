#version 440

const int MAX_INSTANCES = 512;

// Light structure
struct PointLight
{
    vec4 position;
    vec4 color;
};

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
    PointLight lights[MAX_INSTANCES];
};

// Vertex data
in vec3 vPos;

// Per-fragment outputs
layout(location = 0) flat out vec4 lightPos;
layout(location = 1) flat out vec4 lightColor;

void main()
{
    // Grab the current light instance
    PointLight light = lights[gl_InstanceID];

    // Scale vertex position by instance radius
    gl_Position = viewProj * vec4(light.position.xyz + (vPos * light.position.w), 1.0);

    // Send light data to fragment shader
    lightPos = light.position;
    lightColor = light.color;
}