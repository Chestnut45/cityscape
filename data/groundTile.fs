#version 150

uniform sampler2D tex;

// Varying inputs
in vec3 fragPos;
in vec3 normal;
in vec2 texCoords;

// Geometry buffer outputs
out vec3 gPos;
out vec3 gNorm;
out vec4 gColorSpec;

void main()
{
    // Store geometry data in gBuffer
    gPos = fragPos;
    gNorm = normalize(normal);
    gColorSpec = texture(tex, texCoords);
}