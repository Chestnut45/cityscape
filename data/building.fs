#version 150

// Building atlas texture sampler
uniform sampler2D buildingAtlas;

in vec3 normal;
in vec2 texCoords;

out vec4 finalColor;

void main()
{
    finalColor = texture(buildingAtlas, texCoords);
}