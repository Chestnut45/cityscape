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

        float w = texture->GetWidth();
        float h = texture->GetHeight();
        tileSizeNormalized = glm::vec2((float)TILE_SIZE / w, (float)TILE_SIZE / h);
    }
    refCount++;

    // Clamp to safe input values
    stories = std::clamp(stories, 1, MAX_STORIES);
    variant = std::clamp(variant, 0, NUM_VARIANTS);

    // Only need to calculate these once
    float halfSize = 0.5f * storySize;

    // Generate the first story

    // Decide texOffset based on orientation (only matters for door placement)
    if (orientation == Orientation::North)
    {
        AddWall(Orientation::North, TexOffset::Door, variant, 0, halfSize, storySize);
        AddWall(Orientation::East, TexOffset::Wall, variant, 0, halfSize, storySize);
        AddWall(Orientation::South, TexOffset::Wall, variant, 0, halfSize, storySize);
        AddWall(Orientation::West, TexOffset::Wall, variant, 0, halfSize, storySize);
    }

    for (const auto& v : indices)
    {
        std::cout << v << std::endl;
    }

    // Generate each additional story's vertex data
    for (int i = 0; i < stories; i++)
    {
        // Calculate story offset
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
    std::transform(indices.cbegin(), indices.cend(), offsetIndices.begin(), [](GLuint original) { return original + indexCount; });

    // Write offset index data
    ebo->Write(offsetIndices.data(), indBytes);

    // Increase static counters
    vertexCount += vertices.size();
    indexCount += indices.size();
    drawCount++;
}

// Flushes all buildings drawn since last flush
void Building::Flush()
{
    // Only flush if we have drawn at least once building
    if (drawCount == 0) return;

    // Ensure all buffer writes are flushed
    vbo->Flush();
    ebo->Flush();
    
    // Bind relevant resources
    shader->Use();
    texture->Bind();
    vao->Bind();

    // Issue draw call
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset static counters
    vertexCount = 0;
    indexCount = 0;
    drawCount = 0;
}

// Constructs a wall with the given parameters
void Building::AddWall(Orientation dir, TexOffset type, int variant, int story, float halfSize, float storySize)
{
    // Calculate texture offsets for upper left vertex of the wall
    float xTexOffs = (float)type * tileSizeNormalized.x;
    float yTexOffs = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;

    // Offset indices by current number of verts
    GLuint n = vertices.size();

    switch (dir)
    {
        // Construct a global north facing wall (Z-)
        case Orientation::North:
            vertices.push_back({halfSize, storySize, -halfSize, 0.0f, 0.0f, -1.0f, xTexOffs, yTexOffs});
            vertices.push_back({-halfSize, storySize, -halfSize, 0.0f, 0.0f, -1.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
            vertices.push_back({halfSize, 0, -halfSize, 0.0f, 0.0f, -1.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
            vertices.push_back({-halfSize, 0, -halfSize, 0.0f, 0.0f, -1.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});
            break;
        
        // Construct a global east facing wall (X+)
        case Orientation::East:
            vertices.push_back({halfSize, storySize, halfSize, 1.0f, 0.0f, 0.0f, xTexOffs, yTexOffs});
            vertices.push_back({halfSize, storySize, -halfSize, 1.0f, 0.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
            vertices.push_back({halfSize, 0, halfSize, 1.0f, 0.0f, 0.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
            vertices.push_back({halfSize, 0, -halfSize, 1.0f, 0.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});
            break;
        
        // Construct a global south facing wall (Z+)
        case Orientation::South:
            vertices.push_back({-halfSize, storySize, halfSize, 0.0f, 0.0f, 1.0f, xTexOffs, yTexOffs});
            vertices.push_back({halfSize, storySize, halfSize, 0.0f, 0.0f, 1.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
            vertices.push_back({-halfSize, 0, halfSize, 0.0f, 0.0f, 1.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
            vertices.push_back({halfSize, 0, halfSize, 0.0f, 0.0f, 1.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});
            break;
        
        // Construct a global west facing wall (X-)
        case Orientation::West:
            vertices.push_back({-halfSize, storySize, -halfSize, -1.0f, 0.0f, 0.0f, xTexOffs, yTexOffs});
            vertices.push_back({-halfSize, storySize, halfSize, -1.0f, 0.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
            vertices.push_back({-halfSize, 0, -halfSize, -1.0f, 0.0f, 0.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
            vertices.push_back({-halfSize, 0, halfSize, -1.0f, 0.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});
            break;
    }

    // Add the indices for the wall
    indices.push_back(n);
    indices.push_back(n + 2);
    indices.push_back(n + 1);
    indices.push_back(n + 1);
    indices.push_back(n + 2);
    indices.push_back(n + 3);
}