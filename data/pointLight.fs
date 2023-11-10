#version 150

// Camera uniform block
layout(std140) uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
    vec2 resolution;
};

// Geometry buffer textures
uniform sampler2D gPos;
uniform sampler2D gNorm;
uniform sampler2D gColorSpec;

// Vertex inputs
flat in vec4 lightPos;
flat in vec4 lightColor;

// Final color
out vec4 outColor;

void main()
{
    // Calculate texture coordinates from screen resolution and fragment coordinates
    vec2 texCoords = gl_FragCoord.xy / resolution;

    // Grab data from geometry buffer
    vec3 fragPos = texture(gPos, texCoords).xyz;
    vec3 fragNorm = texture(gNorm, texCoords).xyz;
    vec3 fragAlbedo = texture(gColorSpec, texCoords).xyz;

    // Constant material properties
    float specularStrength = 0.45;
    float shininess = 32;

    // Initial values
    vec3 result = vec3(0);
    vec3 lightDir = normalize(lightPos.xyz - fragPos);

    // Diffuse calculations
    vec3 diffuse =  max(dot(fragNorm, lightDir), 0) * lightColor.rgb;

    // Specular calculations
    vec3 halfDir = normalize(lightDir + (normalize(cameraPos.xyz - fragPos)));
    float spec = pow(max(dot(fragNorm, halfDir), 0), shininess);
    vec3 specular = specularStrength * spec * lightColor.rgb;

    // Attenuation
    float radius = lightPos.w;
    float dist = distance(fragPos, lightPos.xyz);
    float attenuation = clamp(1.0 - dist / radius, 0.0, 1.0);
    attenuation *= attenuation;

    // Set final output color
    outColor = vec4((diffuse + specular) * attenuation, 1.0);
}