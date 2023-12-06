#version 440

// Varying inputs
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoords1;
layout(location = 4) in vec2 texCoords2;

// Geometry buffer outputs
layout(location = 0) out vec3 gPos;
layout(location = 1) out vec3 gNorm;
layout(location = 2) out vec4 gColorSpec;

void main()
{
    // Store data into geometry buffer
    gPos = fragPos;

    // NOTE: Since all point lights are directly below the bulbs,
    // we can just pretend the normals all face down to get more
    // even lighting across the surface of the bulb.
    gNorm = vec3(0.0, -1.0, 0.0);
    gColorSpec = vec4(1.0);
}