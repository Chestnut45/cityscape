#version 150

// Building atlas texture sampler
uniform sampler2D buildingAtlas;

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
    // Store data into geometry buffer
    gPos = fragPos;
    gNorm = normalize(normal);
    gColorSpec = texture(buildingAtlas, texCoords);
}