#version 440

// Texture samplers
layout(binding = 0) uniform sampler2D colorSpecMap;

// Varying inputs
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords1;
layout(location = 3) in vec2 texCoords2;

// Geometry buffer outputs
layout(location = 0) out vec3 gPos;
layout(location = 1) out vec3 gNorm;
layout(location = 2) out vec4 gColorSpec;

void main()
{
    // Store data into geometry buffer
    gPos = fragPos;
    gNorm = normalize(normal);
    gColorSpec = vec4(1.0);//texture(colorMap, texCoords1);
}