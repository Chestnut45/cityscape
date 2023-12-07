#version 440

const float SNOW_DRIFTINESS = 0.6;

in vec3 fragPos;
in vec4 fragColor;
in vec3 fragNorm;

// Geometry buffer outputs
layout(location = 0) out vec3 gPos;
layout(location = 1) out vec3 gNorm;
layout(location = 2) out vec4 gColorSpec;

void main()
{
    // Store geometry data in gBuffer
    gPos = fragPos;
    gNorm = fragNorm;
    gColorSpec = vec4(1.0, 1.0, 1.0, 0.2);
}