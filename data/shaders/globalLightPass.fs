#version 440

const float MIN_SHADOW_BIAS = 0.0000;
const float MAX_SHADOW_BIAS = 0.0008;

// Light structure
struct DirectionalLight
{
    vec4 position;
    vec4 direction;
    vec4 color;
};

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    vec2 resolution;
};

// Lighting uniform block
layout(std140, binding = 2) uniform GlobalLightBlock
{
    DirectionalLight globalLight;
    float ambient;
};

// Light space uniform block
layout(std140, binding = 5) uniform LightSpaceBlock
{
    mat4 lightViewProj;
};

layout(binding = 0) uniform sampler2D gPos;
layout(binding = 1) uniform sampler2D gNorm;
layout(binding = 2) uniform sampler2D gColorSpec;
layout(binding = 3) uniform sampler2D shadowMap;

// Texture coordinates
in vec2 texCoords;

// Final output
out vec4 outColor;

void main()
{
    // Grab data from geometry buffer
    vec3 fragPos = texture(gPos, texCoords).xyz;
    vec3 fragNorm = texture(gNorm, texCoords).xyz;
    vec4 colorSpec = texture(gColorSpec, texCoords);
    vec3 fragAlbedo = colorSpec.rgb;

    // Calculate view direction
    vec3 viewDir = normalize(cameraPos.xyz - fragPos);

    // Directions towards global light
    vec3 lightDir = normalize(-globalLight.direction.xyz);

    // Calculate alignment to light
    float alignment = dot(fragNorm, lightDir);

    // Constant material properties
    float specularStrength = colorSpec.a;
    float shininess = 8;

    // Diffuse lighting
    vec3 diffuse = (max(alignment, 0) * globalLight.color.rgb * globalLight.color.a);

    // Specular reflections
    vec3 sunHalfDir = normalize(lightDir + viewDir);
    float specSun = pow(max(dot(fragNorm, sunHalfDir), 0), shininess);
    vec3 specular = specularStrength * (specSun * globalLight.color.rgb * globalLight.color.a);

    // Transform fragment position to light space and project
    vec4 posLightSpace = lightViewProj * vec4(fragPos, 1.0);
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w * 0.5 + 0.5;
    float closest = texture(shadowMap, projCoords.xy).r;
    float current = projCoords.z;

    // Calculate final shadow value
    // NOTE: Ensures we don't shadow any stuff facing away from the light
    float bias = max(MIN_SHADOW_BIAS, MAX_SHADOW_BIAS * (1.0 - alignment));
    float shadow = alignment < 0.001 ? 0.0 : (current < 1.0 && current - bias > closest) ? max(min(globalLight.color.a, 0.8), 0.2) : 0.0;

    // Final color composition
    outColor = vec4(((ambient + diffuse) * fragAlbedo + specular) * (1.0 - shadow), 1.0);
}