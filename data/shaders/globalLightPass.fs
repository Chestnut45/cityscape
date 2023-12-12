#version 440

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

// Geometry buffer textures
layout(binding = 0) uniform sampler2D gPos;
layout(binding = 1) uniform sampler2D gNorm;
layout(binding = 2) uniform sampler2D gColorSpec;
layout(binding = 3) uniform sampler2D shadowMap;

// Texture coordinates
in vec2 texCoords;

// Final output
out vec4 outColor;

// Global light transform matrix
uniform mat4 lightViewProj;

void main()
{
    // Grab data from geometry buffer
    vec3 fragPos = texture(gPos, texCoords).xyz;
    vec3 fragNorm = texture(gNorm, texCoords).xyz;
    vec4 colorSpec = texture(gColorSpec, texCoords);
    vec3 fragAlbedo = colorSpec.rgb;

    // Constant material properties
    float specularStrength = colorSpec.a;
    float shininess = 32;

    // Directions towards global light
    vec3 lightDir = normalize(-globalLight.direction.xyz);

    // Diffuse lighting
    vec3 diffuse = (max(dot(fragNorm, lightDir), 0) * globalLight.color.rgb * globalLight.color.a);

    // Specular reflections
    vec3 viewDir = normalize(cameraPos.xyz - fragPos);
    vec3 sunHalfDir = normalize(lightDir + viewDir);
    float specSun = pow(max(dot(fragNorm, sunHalfDir), 0), shininess);
    vec3 specular = specularStrength * (specSun * globalLight.color.rgb * globalLight.color.a);

    // Calculate shadow
    vec4 posLightSpace = lightViewProj * vec4(fragPos, 1.0);
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w * 0.5 + 0.5;
    float closest = texture(shadowMap, projCoords.xy).r;
    float current = projCoords.z;
    float bias = max(0.0001, 0.0005 * (1.0 - dot(fragNorm, lightDir)));
    float shadow = dot(fragNorm, lightDir) < 0.01 ? 0.0 : (current < 1.0 && current - bias > closest) ? min(globalLight.color.a, 0.8) : 0.0;

    // Final color composition
    outColor = vec4(((ambient + diffuse) * fragAlbedo + specular) * (1.0 - shadow), 1.0);
}