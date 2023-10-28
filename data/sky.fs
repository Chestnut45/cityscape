#version 150

uniform samplerCube dayCube;
uniform samplerCube nightCube;

uniform float time;

// Direction vector
in vec3 texCoords;

out vec4 finalColor;

void main()
{
    // Sample both skyboxes directly
    vec4 dayTexel = texture(dayCube, texCoords);
    vec4 nightTexel = texture(nightCube, texCoords);

    // Blend both texels based on normalized time of day
    finalColor = dayTexel * (1 - time) + nightTexel * time;
}