#include "groundtile.hpp"

static const int TILE_SIZE = 16;

// Static ground tile vertex data (pos, normal, uv)
static const Phi::VertexPosNormUv GROUND_VERTICES[] =
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
GroundTile::GroundTile(const glm::ivec2& id) : position(id.x * TILE_SIZE, 0, id.y * TILE_SIZE, 1)
{
    // If first tile created
    if (refCount == 0)
    {
        // Initialize static resources
        texture = new Phi::Texture2D("data/textures/cityBlockGround.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, true);
        vbo = new Phi::GPUBuffer(Phi::BufferType::Static, sizeof(GROUND_VERTICES), GROUND_VERTICES);
        ebo = new Phi::GPUBuffer(Phi::BufferType::Static, sizeof(GROUND_INDICES), GROUND_INDICES);
        vao = new Phi::VertexAttributes(Phi::VertexFormat::POS_NORM_UV, vbo, ebo);

        instanceUBO = new Phi::GPUBuffer(Phi::BufferType::Dynamic, sizeof(glm::vec4) * MAX_INSTANCES);

        // Load the default shader
        shader = new Phi::Shader();
        shader->LoadShaderSource(GL_VERTEX_SHADER, "data/shaders/groundTile.vs");
        shader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/shaders/groundTile.fs");
        shader->Link();
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

    instanceUBO->Sync();

    // Increase counter and write instance position to buffer
    drawCount++;
    instanceUBO->Write(position);
}

// Flushes all grounds drawn since the last flush
void GroundTile::FlushDrawCalls()
{
    // Only flush if there is something to render
    if (drawCount == 0) return;

    // Bind resources
    instanceUBO->BindBase(GL_UNIFORM_BUFFER, 1);
    vao->Bind();
    texture->Bind();
    shader->Use();

    // Issue draw call
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, drawCount);
    glBindVertexArray(0);
    
    // Lock the buffer and reset the pointer
    instanceUBO->Lock();
    instanceUBO->SetOffset(0);

    // Reset counter
    drawCount = 0;
}