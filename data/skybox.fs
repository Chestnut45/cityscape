#version 150

// Light structure
struct DirectionalLight
{
    vec4 position;
    vec4 direction;
    vec4 color;
};

// Lighting uniform block
layout(std140) uniform GlobalLightBlock
{
    DirectionalLight sun;
    DirectionalLight moon;
    float ambient;
};

// Normalized TOD: 0 = noon, 1 = midnight
uniform float time;

// Skybox texture samplers
uniform samplerCube dayCube;
uniform samplerCube nightCube;

// Direction vector
in vec3 texCoords;

out vec4 finalColor;

void main()
{
    // Sample both skyboxes directly
    vec4 dayTexel = texture(dayCube, texCoords);
    vec4 nightTexel = texture(nightCube, texCoords);

    // Calculate influence on sky color
    vec4 celestialBodyInfluence = mix(sun.color, moon.color, time) * mix(ambient, 0.0, time);

    // Blend both texels based on normalized time of day and influence from sun + moon
    finalColor = mix(dayTexel, nightTexel, time) + celestialBodyInfluence;
}