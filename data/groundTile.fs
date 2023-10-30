#version 150

uniform sampler2D tex;

in vec2 texCoords;

out vec4 finalColor;

void main()
{
    finalColor = texture(tex, texCoords);
}