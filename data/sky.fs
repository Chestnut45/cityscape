#version 150

uniform samplerCube dayCube;
uniform samplerCube nightCube;

// Direction vector
in vec3 texCoords;

out vec4 finalColor;

void main()
{
    // Sample the skybox directly for final color
    finalColor = texture(nightCube, texCoords);
}