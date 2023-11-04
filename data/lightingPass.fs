#version 150

const int MAX_LIGHTS = 450;

// Light structures
struct DirectionalLight
{
    vec3 direction;
    vec3 color;
    float ambient;
    float specular;
    float diffuse;
};

struct PointLight
{
    vec3 position;
    vec3 color;
    float ambient;
    float specular;
    float diffuse;
};

// Camera uniform block
layout(std140) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
};

// Lighting uniforms
uniform DirectionalLight sun;

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
    float specularStrength = 0.5;
    float shininess = 32;

    // Initial values
    vec3 result = vec3(0);
    vec3 lightDir = normalize(-sun.direction.xyz);
    vec3 lightColor = sun.color.rgb;

    // Diffuse lighting
    vec3 diffuse = max(dot(fragNorm, lightDir), 0) * lightColor;

    // Specular reflections
    vec3 halfDir = normalize(lightDir + (normalize(cameraPos.xyz - fragPos)));
    float spec = pow(max(dot(fragNorm, halfDir), 0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Final color composition
    result += (sun.ambient + diffuse + specular) * fragAlbedo;
    outColor = vec4(result, 1);
}