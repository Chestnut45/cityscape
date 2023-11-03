#version 150

// Texture coordinates
in vec2 texCoords;

// Final output
out vec4 fragColor;

// Geometry buffer textures
uniform sampler2D gPos;
uniform sampler2D gNorm;
uniform sampler2D gColorSpec;

void main()
{
    fragColor = texture(gColorSpec, texCoords);
}