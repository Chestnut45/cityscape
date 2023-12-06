#include "lights.hpp"

// Constructor
DirectionalLight::DirectionalLight(const glm::vec4& pos, const glm::vec4& dir, const glm::vec4& col)
    : position(pos), direction(dir), color(col)
{

}

// Destructor
DirectionalLight::~DirectionalLight()
{

}

// Constructor
PointLight::PointLight(const glm::vec4& pos, const glm::vec4& col) : position(pos), color(col)
{
    // Initialize static resources on first instance created
    if (refCount == 0)
    {
        // Vertex data
        vbo = new Phi::GPUBuffer(Phi::BufferType::Static, sizeof(Phi::Icosphere::ICOSPHERE_VERTICES), Phi::Icosphere::ICOSPHERE_VERTICES);
        ebo = new Phi::GPUBuffer(Phi::BufferType::Static, sizeof(Phi::Icosphere::ICOSPHERE_INDICES), Phi::Icosphere::ICOSPHERE_INDICES);
        vao = new Phi::VertexAttributes(Phi::VertexFormat::POS, vbo, ebo);

        shader = new Phi::Shader();
        shader->LoadShaderSource(GL_VERTEX_SHADER, "data/shaders/pointLight.vs");
        shader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/shaders/pointLight.fs");
        shader->Link();

        // Instance buffer data
        // 512 * 2 * sizeof(glm::vec4) = 16,384 = minimum UBO limit required by OpenGL
        instanceUBO = new Phi::GPUBuffer(Phi::BufferType::Dynamic, sizeof(glm::vec4) * 2 * MAX_INSTANCES);
    }

    refCount++;
}

// Destructor
PointLight::~PointLight()
{
    refCount--;

    // Destroy static resources on last instance destroyed
    if (refCount == 0)
    {
        delete vbo;
        delete ebo;
        delete vao;
        delete shader;
        delete instanceUBO;
    }
}

// Draws into instance buffer, flushing if it is full
void PointLight::Draw()
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
    instanceUBO->Write(color);
}

void PointLight::FlushDrawCalls()
{
    // Only flush if there is something to render
    if (drawCount == 0) return;

    // Bind objects
    instanceUBO->BindBase(GL_UNIFORM_BUFFER, 1);
    vao->Bind();
    shader->Use();

    // Issue draw call
    glDrawElementsInstanced(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0, drawCount);
    glBindVertexArray(0);

    // Lock buffer and reset pointer
    instanceUBO->Lock();
    instanceUBO->SetOffset(0);

    // Reset counter
    drawCount = 0;
}