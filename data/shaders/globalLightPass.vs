#version 440

// Fullscreen triangle vertices
// Saves helper fragments that would be generated at the seam of a quad
const vec2 verts[3] =
{
    vec2(-1, -1),
    vec2(3, -1),
    vec2(-1, 3)
};

out vec2 texCoords;

void main()
{
    gl_Position = vec4(verts[gl_VertexID], 0, 1);
    texCoords = gl_Position.xy * 0.5 + 0.5;
}