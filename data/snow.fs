#version 440

in vec3 fragPos;
in vec3 normal;

// Geometry buffer outputs
layout(location = 0) out vec3 gPos;
layout(location = 1) out vec3 gNorm;
layout(location = 2) out vec4 gColorSpec;

void main()
{
    // Store geometry data in gBuffer
    gPos = fragPos;
    gNorm = normal;
    gColorSpec = vec4(1.0);
}