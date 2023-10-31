#include "building.hpp"

// Main constructor
Building::Building(const glm::ivec3 &pos, int stories, float storySize, int variant, FeatureFlags features, Orientation orientation)
{
    // Initialize static resources if first instance
    if (refCount == 0)
    {
        texture = new Texture2D("data/buildingAtlas.png");
        vbo = new GPUBuffer(BufferType::DynamicVertex, sizeof(VertexPosNormUv) * MAX_VERTICES);
        ebo = new GPUBuffer(BufferType::DynamicIndex, sizeof(GLuint) * MAX_INDICES);
        vao = new VertexAttributes(VertexFormat::POS_NORM_UV, vbo, ebo);

        shader = new Shader();
        shader->LoadShaderSource(GL_VERTEX_SHADER, "data/building.vs");
        shader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/building.fs");
        shader->Link();

        shader->Use();
        shader->BindUniformBlock("CameraBlock", 0);
        shader->SetUniform("buildingAtlas", 0);
    }
    refCount++;

    // Clamp to safe input values
    stories = std::clamp(stories, 1, MAX_STORIES);
    variant = std::clamp(variant, 0, NUM_VARIANTS);

    // Retrieve texture dimensions
    float w = texture->GetWidth();
    float h = texture->GetHeight();

    // Only need to calculate these once
    float halfSize = 0.5f * storySize;
    float tileSizeNormalized = (float)TILE_SIZE / w;
    float yOffset = 0;

    // Initialize texture atlas offsets
    glm::vec2 texOffset = {0, variant * (h / NUM_VARIANTS)};

    // Generate the first story

    // Decide texOffset based on orientation (only matters for door placement)
    if (orientation == Orientation::North)
        texOffset.x = (int)TexOffset::Door * w;
    else
        texOffset.x = (int)TexOffset::Side * w;

    // North wall (Z-)
    vertices.push_back({halfSize, storySize, -halfSize, 0.0f, 0.0f, -1.0f, texOffset.x, texOffset.y});
    vertices.push_back({-halfSize, storySize, -halfSize, 0.0f, 0.0f, -1.0f, texOffset.x + tileSizeNormalized, texOffset.y});
    vertices.push_back({halfSize, 0, -halfSize, 0.0f, 0.0f, -1.0f, texOffset.x, texOffset.y + tileSizeNormalized});
    vertices.push_back({-halfSize, 0, -halfSize, 0.0f, 0.0f, -1.0f, texOffset.x + tileSizeNormalized, texOffset.y + tileSizeNormalized});
    AddQuad();

    if (orientation == Orientation::East)
        texOffset.x = (int)TexOffset::Door * w;
    else
        texOffset.x = (int)TexOffset::Side * w;

    // East wall (X+)
    vertices.push_back({halfSize, storySize, halfSize, 1.0f, 0.0f, 0.0f, texOffset.x, texOffset.y});
    vertices.push_back({halfSize, storySize, -halfSize, 1.0f, 0.0f, 0.0f, texOffset.x + tileSizeNormalized, texOffset.y});
    vertices.push_back({halfSize, 0, halfSize, 1.0f, 0.0f, 0.0f, texOffset.x, texOffset.y + tileSizeNormalized});
    vertices.push_back({halfSize, 0, -halfSize, 1.0f, 0.0f, 0.0f, texOffset.x + tileSizeNormalized, texOffset.y + tileSizeNormalized});
    AddQuad();

    if (orientation == Orientation::South)
        texOffset.x = (int)TexOffset::Door * w;
    else
        texOffset.x = (int)TexOffset::Side * w;

    // South wall (Z+)
    vertices.push_back({-halfSize, storySize, halfSize, 0.0f, 0.0f, 1.0f, texOffset.x, texOffset.y});
    vertices.push_back({halfSize, storySize, halfSize, 0.0f, 0.0f, 1.0f, texOffset.x + tileSizeNormalized, texOffset.y});
    vertices.push_back({-halfSize, 0, halfSize, 0.0f, 0.0f, 1.0f, texOffset.x, texOffset.y + tileSizeNormalized});
    vertices.push_back({halfSize, 0, halfSize, 0.0f, 0.0f, 1.0f, texOffset.x + tileSizeNormalized, texOffset.y + tileSizeNormalized});
    AddQuad();

    if (orientation == Orientation::West)
        texOffset.x = (int)TexOffset::Door * w;
    else
        texOffset.x = (int)TexOffset::Side * w;

    // West wall (X-)
    vertices.push_back({-halfSize, storySize, -halfSize, -1.0f, 0.0f, 0.0f, texOffset.x, texOffset.y});
    vertices.push_back({-halfSize, storySize, halfSize, -1.0f, 0.0f, 0.0f, texOffset.x + tileSizeNormalized, texOffset.y});
    vertices.push_back({-halfSize, 0, -halfSize, -1.0f, 0.0f, 0.0f, texOffset.x, texOffset.y + tileSizeNormalized});
    vertices.push_back({-halfSize, 0, halfSize, -1.0f, 0.0f, 0.0f, texOffset.x + tileSizeNormalized, texOffset.y + tileSizeNormalized});
    AddQuad();

    // Generate each additional story's vertex data
    for (int i = 0; i < stories; i++)
    {
        // Calculate story offset
        yOffset += storySize;
    }

    // Generate the final story + roof

    // Store final size of each local buffer
    vertBytes = vertices.size() * sizeof(VertexPosNormUv);
    indBytes = indices.size() * sizeof(GLuint);

    // Reserve proper space for offsets while rendering
    offsetIndices.reserve(indices.size());
}

// Cleanup
Building::~Building()
{
    refCount--;
    if (refCount == 0)
    {
        // Cleanup static resources
        delete texture;
        delete vbo;
        delete ebo;
        delete vao;
        delete shader;
    }
}

// Draws this building into the static buffer
void Building::Draw()
{
    // Flush if either of the static buffers are full
    if (!(vbo->CanWrite(vertBytes) && ebo->CanWrite(indBytes)))
    {
        Flush();
    }

    // Write vertex data
    vbo->Write(vertices.data(), vertBytes);

    // Recalculate offset indices
    // std::transform(indices.cbegin(), indices.cend(), offsetIndices.begin(), [](GLuint original) { return original + indexCount; });

    offsetIndices.clear();
    for (const GLuint& s : indices)
    {
        offsetIndices.push_back(s + indexCount);
    }

    // Write offset index data
    ebo->Write(offsetIndices.data(), indBytes);

    // Increase static counters
    vertexCount += vertices.size();
    indexCount += indices.size();
}

// Flushes all buildings drawn since last flush
void Building::Flush()
{
    // Ensure all buffer writes are flushed
    vbo->Flush();
    ebo->Flush();
    
    // Bind relevant resources
    shader->Use();
    texture->Bind();
    vao->Bind();

    // Issue draw call
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_BYTE, 0);
    glBindVertexArray(0);

    // Reset static counters
    vertexCount = 0;
    indexCount = 0;
}

// Adds a quad from the last 4 vertices added
void Building::AddQuad()
{
    indices.push_back(vertices.size());
    indices.push_back(vertices.size() + 2);
    indices.push_back(vertices.size() + 1);
    indices.push_back(vertices.size() + 1);
    indices.push_back(vertices.size() + 2);
    indices.push_back(vertices.size() + 3);
}