#version 440

// Texture samplers
layout(binding = 0) uniform sampler2D colorMap;
layout(binding = 2) uniform sampler2D specMap;

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
    gNorm = normalize(normal);
    gColorSpec = texture(colorMap, texCoords1);
}