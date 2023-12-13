#version 440

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    vec2 resolution;
};

// Geometry buffer textures
layout(binding = 0) uniform sampler2D gPos;
layout(binding = 1) uniform sampler2D gNorm;
layout(binding = 2) uniform sampler2D gColorSpec;

// Vertex inputs
layout(location = 0) flat in vec4 lightPos;
layout(location = 1) flat in vec4 lightColor;

// Final color
out vec4 outColor;

void main()
{
    // Calculate texture coordinates from screen resolution and fragment coordinates
    vec2 texCoords = gl_FragCoord.xy / resolution;

    // Grab data from geometry buffer
    vec3 fragPos = texture(gPos, texCoords).xyz;
    vec3 fragNorm = texture(gNorm, texCoords).xyz;
    vec4 colorSpec = texture(gColorSpec, texCoords);
    vec3 fragAlbedo = colorSpec.rgb;

    // Constant material properties
    float specularStrength = colorSpec.a;
    float shininess = 128; // Again, shouldn't be constant, but it made little difference for this project

    // Initial values
    vec3 lightDir = normalize(lightPos.xyz - fragPos);
    vec3 viewDir = normalize(cameraPos.xyz - fragPos);

    // Diffuse lighting
    float diffuseAmount = max(dot(fragNorm, lightDir), 0);
    vec3 diffuse =  diffuseAmount * lightColor.rgb * fragAlbedo;

    // Specular reflections
    vec3 halfDir = normalize(lightDir + viewDir);
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