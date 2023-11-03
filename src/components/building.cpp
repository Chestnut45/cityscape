#include "building.hpp"

// Main constructor
Building::Building(const glm::ivec3 &pos, int stories, int blocks, int variant, Feature features, Orientation orientation)
{
    // Initialize static resources if first instance
    if (refCount == 0)
    {
        texture = new Texture2D("data/buildingAtlas.png", GL_NEAREST);
        vbo = new GPUBuffer(BufferType::DynamicVertex, sizeof(VertexPosNormUv) * MAX_VERTICES);
        ebo = new GPUBuffer(BufferType::DynamicIndex, sizeof(GLuint) * MAX_INDICES);
        vao = new VertexAttributes(VertexFormat::POS_NORM_UV, vbo, ebo);

        shader = new Shader();
        shader->LoadShaderSource(GL_VERTEX_SHADER, "data/building.vs");
        shader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/building.fs");

        // Set gBuffer fragment output locations
        glBindFragDataLocation(shader->GetProgramID(), 0, "gPos");
        glBindFragDataLocation(shader->GetProgramID(), 1, "gNorm");
        glBindFragDataLocation(shader->GetProgramID(), 2, "gColorSpec");
        
        shader->Link();
        shader->Use();
        shader->BindUniformBlock("CameraBlock", 0);
        shader->SetUniform("buildingAtlas", 0);

        // Calculate normalized tile size for atlas offsets
        float w = texture->GetWidth();
        float h = texture->GetHeight();
        tileSizeNormalized = glm::vec2((float)TILE_SIZE / w, (float)TILE_SIZE / h);
    }
    refCount++;

    // Clamp to safe input values
    stories = std::clamp(stories, 1, MAX_STORIES);
    variant = std::clamp(variant, 0, NUM_VARIANTS);

    // Generate the first story
    // Place door depending on facing direction
    switch (orientation)
    {
        case Orientation::North:
            AddFace(Orientation::North, TexOffset::Door, variant, 0, blocks);
            AddFace(Orientation::East, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::South, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::West, TexOffset::Wall, variant, 0, blocks);
            break;
        
        case Orientation::East:
            AddFace(Orientation::North, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::East, TexOffset::Door, variant, 0, blocks);
            AddFace(Orientation::South, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::West, TexOffset::Wall, variant, 0, blocks);
            break;
        
        case Orientation::South:
            AddFace(Orientation::North, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::East, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::South, TexOffset::Door, variant, 0, blocks);
            AddFace(Orientation::West, TexOffset::Wall, variant, 0, blocks);
            break;
        
        case Orientation::West:
            AddFace(Orientation::North, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::East, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::South, TexOffset::Wall, variant, 0, blocks);
            AddFace(Orientation::West, TexOffset::Door, variant, 0, blocks);
            break;
    }

    // Generate each additional story's vertex data
    int currentStoryBlocks = blocks;
    for (int i = 1; i < stories; i++)
    {
        AddFace(Orientation::North, RandomWallType(), variant, i, currentStoryBlocks);
        AddFace(Orientation::East, RandomWallType(), variant, i, currentStoryBlocks);
        AddFace(Orientation::South, RandomWallType(), variant, i, currentStoryBlocks);
        AddFace(Orientation::West, RandomWallType(), variant, i, currentStoryBlocks);

        // If not the final story
        if (i != stories - 1)
        {
            // Step?
            if (stepDist(rng) == 0 && currentStoryBlocks > 1)
            {
                // Generate the roof
                AddFace(Orientation::Up, TexOffset::Roof, variant, i, currentStoryBlocks);
                currentStoryBlocks--;
            }
        }
    }

    // Generate the final roof
    AddFace(Orientation::Up, TexOffset::Roof, variant, stories - 1, currentStoryBlocks);

    // Iterate all vertices and add world position offset after generation is complete
    for (auto& v : vertices)
    {
        v.x += pos.x;
        v.y += pos.y;
        v.z += pos.z;
    }

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
    std::transform(indices.cbegin(), indices.cend(), offsetIndices.begin(), [](GLuint original) { return original + vertexCount; });

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
void Building::AddFace(Orientation dir, TexOffset type, int variant, int story, int blocks)
{
    // Calculate texture offsets for upper left vertex of the wall
    float xTexOffs = (float)type * tileSizeNormalized.x;
    float yTexOffs = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;

    float storySize = 1.0f;
    float halfSize = 0.5f;
    float yPosOffs = storySize * story;

    // Calculate offset based on number of blocks
    float xOffset = -halfSize * (blocks - 1);
    float zOffset = xOffset;

    switch (dir)
    {
        // Construct a global north face (Z-)
        case Orientation::North:
            for (int i = 0; i < blocks; i++)
            {
                GLuint n = vertices.size();

                vertices.push_back({halfSize + xOffset, yPosOffs + storySize, -halfSize * blocks, 0.0f, 0.0f, -1.0f, xTexOffs, yTexOffs});
                vertices.push_back({-halfSize + xOffset, yPosOffs + storySize, -halfSize * blocks, 0.0f, 0.0f, -1.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
                vertices.push_back({halfSize + xOffset, yPosOffs, -halfSize * blocks, 0.0f, 0.0f, -1.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
                vertices.push_back({-halfSize + xOffset, yPosOffs, -halfSize * blocks, 0.0f, 0.0f, -1.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});

                // Add indices
                indices.push_back(n);
                indices.push_back(n + 2);
                indices.push_back(n + 1);
                indices.push_back(n + 1);
                indices.push_back(n + 2);
                indices.push_back(n + 3);

                // Adjust offset
                xOffset += storySize;
            }
            break;
        
        // Construct a global east face (X+)
        case Orientation::East:
            for (int i = 0; i < blocks; i++)
            {
                GLuint n = vertices.size();

                vertices.push_back({halfSize * blocks, yPosOffs + storySize, halfSize + xOffset, 1.0f, 0.0f, 0.0f, xTexOffs, yTexOffs});
                vertices.push_back({halfSize * blocks, yPosOffs + storySize, -halfSize + xOffset, 1.0f, 0.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
                vertices.push_back({halfSize * blocks, yPosOffs, halfSize + xOffset, 1.0f, 0.0f, 0.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
                vertices.push_back({halfSize * blocks, yPosOffs, -halfSize + xOffset, 1.0f, 0.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});

                // Add indices
                indices.push_back(n);
                indices.push_back(n + 2);
                indices.push_back(n + 1);
                indices.push_back(n + 1);
                indices.push_back(n + 2);
                indices.push_back(n + 3);

                // Adjust offset
                xOffset += storySize;
            }
            break;
        
        // Construct a global south face (Z+)
        case Orientation::South:
            for (int i = 0; i < blocks; i++)
            {
                GLuint n = vertices.size();
                
                vertices.push_back({-halfSize + xOffset, yPosOffs + storySize, halfSize * blocks, 0.0f, 0.0f, 1.0f, xTexOffs, yTexOffs});
                vertices.push_back({halfSize + xOffset, yPosOffs + storySize, halfSize * blocks, 0.0f, 0.0f, 1.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
                vertices.push_back({-halfSize + xOffset, yPosOffs, halfSize * blocks, 0.0f, 0.0f, 1.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
                vertices.push_back({halfSize + xOffset, yPosOffs, halfSize * blocks, 0.0f, 0.0f, 1.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});

                // Add indices
                indices.push_back(n);
                indices.push_back(n + 2);
                indices.push_back(n + 1);
                indices.push_back(n + 1);
                indices.push_back(n + 2);
                indices.push_back(n + 3);

                // Adjust offset
                xOffset += storySize;
            }
            
            break;
        
        // Construct a global west face (X-)
        case Orientation::West:
            for (int i = 0; i < blocks; i++)
            {
                GLuint n = vertices.size();

                vertices.push_back({-halfSize * blocks, yPosOffs + storySize, -halfSize + xOffset, -1.0f, 0.0f, 0.0f, xTexOffs, yTexOffs});
                vertices.push_back({-halfSize * blocks, yPosOffs + storySize, halfSize + xOffset, -1.0f, 0.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
                vertices.push_back({-halfSize * blocks, yPosOffs, -halfSize + xOffset, -1.0f, 0.0f, 0.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
                vertices.push_back({-halfSize * blocks, yPosOffs, halfSize + xOffset, -1.0f, 0.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});

                // Add indices
                indices.push_back(n);
                indices.push_back(n + 2);
                indices.push_back(n + 1);
                indices.push_back(n + 1);
                indices.push_back(n + 2);
                indices.push_back(n + 3);

                // Adjust offset
                xOffset += storySize;
            }
            break;

        // Construct a global up face (Y+)
        case Orientation::Up:
            for (int i = 0; i < blocks * blocks; i++)
            {
                GLuint n = vertices.size();

                vertices.push_back({-halfSize + xOffset, yPosOffs + storySize, -halfSize + zOffset, 0.0f, 1.0f, 0.0f, xTexOffs, yTexOffs});
                vertices.push_back({halfSize + xOffset, yPosOffs + storySize, -halfSize + zOffset, 0.0f, 1.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs});
                vertices.push_back({-halfSize + xOffset, yPosOffs + storySize, halfSize + zOffset, 0.0f, 1.0f, 0.0f, xTexOffs, yTexOffs - tileSizeNormalized.y});
                vertices.push_back({halfSize + xOffset, yPosOffs + storySize, halfSize + zOffset, 0.0f, 1.0f, 0.0f, xTexOffs + tileSizeNormalized.x, yTexOffs - tileSizeNormalized.y});

                // Add indices
                indices.push_back(n);
                indices.push_back(n + 2);
                indices.push_back(n + 1);
                indices.push_back(n + 1);
                indices.push_back(n + 2);
                indices.push_back(n + 3);

                // Adjust offset
                if ((i + 1) % blocks == 0)
                {
                    xOffset = -halfSize * (blocks - 1);
                    zOffset += storySize;
                }
                else
                {
                    xOffset += storySize;
                }
            }
            
            break;
    }
}

// Adds a feature to the building
void Building::AddFeature(Feature feature, Orientation orientation)
{
    switch (feature)
    {
        case Feature::Awning:

            // Add vertices based on orientation
            switch (orientation)
            {
                case Orientation::North:

                    break;
                
                case Orientation::East:

                    break;
                
                case Orientation::South:

                    break;
                
                case Orientation::West:

                    break;
            }

            // Add indices
            
            break;
    }
}

// Randomly chooses a wall type (not seeded)
Building::TexOffset Building::RandomWallType() const
{
    return (TexOffset)wallDist(rng);
}