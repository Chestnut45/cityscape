#include "groundtile.hpp"

// Vertex layout: (x, y, z) position only
static const VertexPosNormUv GROUND_VERTICES[] =
{
    {-1.0f, 0.0f, -1.0f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f},
    {1.0f,  0.0f, -1.0f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f},
    {1.0f,  0.0f, 1.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f},
    {-1.0f, 0.0f, 1.0f,     0.0f, 1.0f, 0.0f,   1.0f, 0.0f},
};

static const GLuint GROUND_INDICES[] =
{
    0, 2, 1,
    1, 2, 3
};

// Constructor with initial position
GroundTile::GroundTile(const glm::vec3& pos) : position(pos)
{
    if (refCount == 0)
    {
        // Initialize static resources
        texture = new Texture2D("data/cityBlockGround.png");
        vbo = new GPUBuffer(BufferType::StaticVertex, sizeof(GROUND_VERTICES), GROUND_VERTICES);
        ebo = new GPUBuffer(BufferType::StaticIndex, sizeof(GROUND_INDICES), GROUND_INDICES);
        vao = new VertexAttributes(VertexFormat::POS_NORM_UV, vbo, ebo);
        instanceUBO = new GPUBuffer(BufferType::Uniform, sizeof(glm::vec3) * MAX_INSTANCES);
    }

    refCount++;
}

// Destructor
GroundTile::~GroundTile()
{
    refCount--;

    if (refCount == 0)
    {
        // Cleanup static resources
        delete texture;
        delete vbo;
        delete ebo;
        delete vao;
        delete instanceUBO;
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
    instanceUBO->Write(position);
}

// Flushes all grounds drawn since the last flush
void GroundTile::FlushDrawCalls()
{
    // Only flush if there is something to render
    if (drawCount == 0) return;

    // Flush all buffer writes and bind vao
    instanceUBO->Flush();
    vao->Bind();

    // Issue draw call
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, drawCount);

    // Reset counter
    drawCount = 0;
}