// Camera uniform block
uniform CameraBlock
{
    mat4 viewProj;
    vec4 cameraPos;
};

// Global data
uniform float time;

// Plane object data
uniform mat4 model;
uniform vec4 color;

// Vertex attributes
in vec3 vPos;

// Output to fragment shader
out vec4 fColor;

void main()
{
    // Add y offset based on xz position and time
    vec3 pos = vPos + vec3(0, sin(time + (vPos.x + vPos.z) * 2), 0);

    // Set fragment position
    gl_Position = viewProj * model * vec4(pos, 1.0);

    // Change brightness based on object-space y position after vertical offset
    fColor = color * (pos.y + 1) / 2;
}