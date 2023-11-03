#version 150

// Output texture coordinates
out vec2 texCoords;

void main()
{
    // Generate a fullscreen triangle (saves on extra fragments generated for the inner seam of a quad)
    vec2 pos = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
    texCoords = (pos + 1) * 0.5;
    gl_Position = vec4(pos, 0, 1);
}