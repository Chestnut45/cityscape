#version 150

uniform sampler2D tex;

in vec2 texCoords;

out vec4 finalColor;

void main()
{
    finalColor = vec4(1, 0, 0, 0);//texture(tex, texCoords);
}