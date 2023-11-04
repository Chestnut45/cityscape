#version 150

const int MAX_LIGHTS = 450;

// Light structures
struct DirectionalLight
{
    vec3 direction;
    vec3 color;
};

struct PointLight
{
    vec4 position;
    vec4 color;
};

// Camera uniform block
layout(std140) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
};

// Single directional light
uniform DirectionalLight globalLight;

// Geometry buffer textures
uniform sampler2D gPos;
uniform sampler2D gNorm;
uniform sampler2D gColorSpec;

// Texture coordinates
in vec2 texCoords;

// Final output
out vec4 outColor;

void main()
{
    // Grab data from geometry buffer
    vec3 fragPos = texture(gPos, texCoords).xyz;
    vec3 fragNorm = texture(gNorm, texCoords).xyz;
    vec3 fragAlbedo = texture(gColorSpec, texCoords).xyz;

    // Constants
    float ambient = 0.1;
    float specularStrength = 0.45;
    float shininess = 32;

    vec3 result = vec3(0);
    vec3 lightDir = normalize(-globalLight.direction.xyz);
    vec3 lightColor = globalLight.color.rgb;

    // Diffuse lighting
    vec3 diffuse = max(dot(fragNorm, lightDir), 0) * lightColor;

    // Specular reflections
    vec3 halfDir = normalize(lightDir + (normalize(cameraPos.xyz - fragPos)));
    float spec = pow(max(dot(fragNorm, halfDir), 0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Final color composition
    result += (ambient + diffuse + specular) * fragAlbedo;
    outColor = vec4(result, 1);
}