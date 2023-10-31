#include "groundtile.hpp"

static const int TILE_SIZE = 16;

// Static ground tile vertex data (pos, normal, uv)
static const VertexPosNormUv GROUND_VERTICES[] =
{
    {0.0f,              0.0f, 0.0f,                 0.0f, 1.0f, 0.0f,   0.0f, 1.0f},
    {(float)TILE_SIZE,  0.0f, 0.0f,                 0.0f, 1.0f, 0.0f,   1.0f, 1.0f},
    {0.0f,              0.0f, (float)TILE_SIZE,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f},
    {(float)TILE_SIZE,  0.0f, (float)TILE_SIZE,     0.0f, 1.0f, 0.0f,   1.0f, 0.0f},
};

static const GLuint GROUND_INDICES[] =
{
    0, 2, 1,
    1, 2, 3
};

// Constructor with initial position
GroundTile::GroundTile(const glm::vec2& id) : position(id.x * TILE_SIZE, 0, id.y * TILE_SIZE)
{
    // If first tile created
    if (refCount == 0)
    {
        // Initialize static resources
        texture = new Texture2D("data/cityBlockGround.png");
        vbo = new GPUBuffer(BufferType::StaticVertex, sizeof(GROUND_VERTICES), GROUND_VERTICES);
        ebo = new GPUBuffer(BufferType::StaticIndex, sizeof(GROUND_INDICES), GROUND_INDICES);
        vao = new VertexAttributes(VertexFormat::POS_NORM_UV, vbo, ebo);

        instanceUBO = new GPUBuffer(BufferType::Uniform, sizeof(glm::vec4) * MAX_INSTANCES);

        // Load the default shader
        // TODO: Shader should be passed in... but how for static resource?
        shader = new Shader();
        shader->LoadShaderSource(GL_VERTEX_SHADER, "data/groundTile.vs");
        shader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/groundTile.fs");
        shader->Link();
        shader->Use();
        shader->BindUniformBlock("InstanceBlock", 1);
        shader->BindUniformBlock("CameraBlock", 0);
        shader->SetUniform("tex", 0);
    }

    refCount++;
}

// Destructor
GroundTile::~GroundTile()
{
    refCount--;

    // If last tile destroyed
    if (refCount == 0)
    {
        // Cleanup static resources
        delete texture;
        delete vbo;
        delete ebo;
        delete vao;
        delete instanceUBO;
        delete shader;
    }
}

// Draws into instance buffer, flushing if it is full
void GroundTile::Draw()
{
    // Flush if the buffer is already full
    if (drawCount >= MAX_INSTANCES)
    {
        FlushDrawCalls();
    };

    // Increase counter and write instance position to buffer
    drawCount++;
    instanceUBO->Write(glm::vec4(position, 1));
}

// Flushes all grounds drawn since the last flush
void GroundTile::FlushDrawCalls()
{
    // Only flush if there is something to render
    if (drawCount == 0) return;

    // Flush all buffer writes and bind objects
    instanceUBO->Flush();
    instanceUBO->BindBase(GL_UNIFORM_BUFFER, 1);
    vao->Bind();
    texture->Bind();
    shader->Use();

    // Issue draw call
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, drawCount);
    glBindVertexArray(0);

    // Reset counter
    drawCount = 0;
}