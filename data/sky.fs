#version 150

uniform samplerCube dayCube;
uniform samplerCube nightCube;
uniform float time;

// Direction vector
in vec3 texCoords;

out vec4 finalColor;

void main()
{
    // Sample both skyboxes directly and blend for final color
    finalColor = (texture(dayCube, texCoords) * (1 - time)) + (texture(nightCube, texCoords) * time);
}