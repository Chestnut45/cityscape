#version 150

// Camera uniform block
layout(std140) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
};

// Building atlas texture sampler
uniform sampler2D buildingAtlas;

in vec2 texCoords;

out vec4 finalColor;

void main()
{
    finalColor = texture(buildingAtlas, texCoords);
}