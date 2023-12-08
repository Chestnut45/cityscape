#include "building.hpp"

// Main constructor
Building::Building(const glm::ivec3 &pos, int stories, int baseBlockCount, int variant, Orientation orientation)
{
    // Initialize static resources if first instance
    if (refCount == 0)
    {
        textureAtlas = new Phi::Texture2D("data/textures/buildingAtlas.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, true);
        vbo = new Phi::GPUBuffer(Phi::BufferType::DynamicDoubleBuffer, sizeof(Phi::VertexPosNormUv) * MAX_VERTICES);
        ebo = new Phi::GPUBuffer(Phi::BufferType::DynamicDoubleBuffer, sizeof(GLuint) * MAX_INDICES);
        vao = new Phi::VertexAttributes(Phi::VertexFormat::POS_NORM_UV, vbo, ebo);

        shader = new Phi::Shader();
        shader->LoadShaderSource(GL_VERTEX_SHADER, "data/shaders/building.vs");
        shader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/shaders/building.fs");
        shader->Link();

        // Calculate normalized tile size for atlas offsets
        float w = textureAtlas->GetWidth();
        float h = textureAtlas->GetHeight();
        tileSizeNormalized = glm::vec2((float)TILE_SIZE / w, (float)TILE_SIZE / h);
    }
    refCount++;

    this->pos = pos;

    // Clamp to safe input values
    stories = std::clamp(stories, 1, MAX_STORIES);
    variant = std::clamp(variant, 0, NUM_VARIANTS);

    // Generate the first story
    // Place door depending on facing direction
    AddFace(Orientation::North, orientation == Orientation::North ? TexOffset::Door : TexOffset::Wall, variant, 0, baseBlockCount);
    AddFace(Orientation::East, orientation == Orientation::East ? TexOffset::Door : TexOffset::Wall, variant, 0, baseBlockCount);
    AddFace(Orientation::South, orientation == Orientation::South ? TexOffset::Door : TexOffset::Wall, variant, 0, baseBlockCount);
    AddFace(Orientation::West, orientation == Orientation::West ? TexOffset::Door : TexOffset::Wall, variant, 0, baseBlockCount);

    // Generate each additional story's vertex data
    int currentStoryBlocks = baseBlockCount;
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

    // Store final size of each local buffer
    vertBytes = mesh.GetVertices().size() * sizeof(Phi::VertexPosNormUv);
    indBytes = mesh.GetIndices().size() * sizeof(GLuint);

    // Reserve proper space for offsets while rendering
    offsetIndices.reserve(mesh.GetIndices().size());
}

// Cleanup
Building::~Building()
{
    refCount--;
    if (refCount == 0)
    {
        // Cleanup static resources
        delete textureAtlas;
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
        FlushDrawCalls();
    }

    // Ensure we aren't in the process of rendering buildings from this section of the buffer already
    if (drawCount == 0)
    {
        vbo->Sync();
        ebo->Sync();
    }

    // Recalculate offset indices
    std::transform(mesh.GetIndices().cbegin(), mesh.GetIndices().cend(), offsetIndices.begin(), [](GLuint original) { return original + vertexCount; });

    // Write vertex data
    vbo->Write(mesh.GetVertices().data(), vertBytes);

    // Write offset index data
    ebo->Write(offsetIndices.data(), indBytes);

    // Increase static counters
    vertexCount += mesh.GetVertices().size();
    indexCount += mesh.GetIndices().size();
    drawCount++;
}

// Flushes all buildings drawn since last flush
void Building::FlushDrawCalls()
{
    // Only flush if we have drawn at least once building
    if (drawCount == 0) return;
    
    // Bind relevant resources
    shader->Use();
    textureAtlas->Bind();
    vao->Bind();

    // Issue draw call
    glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT,
        (void*)((size_t)ebo->GetSize() * ebo->GetCurrentSection()),
        MAX_VERTICES * vbo->GetCurrentSection());
    
    glBindVertexArray(0);
    
    // Insert a fence sync
    vbo->Lock();
    ebo->Lock();

    // Needed for double-buffering
    vbo->SwapSections();
    ebo->SwapSections();

    // Reset static counters
    vertexCount = 0;
    indexCount = 0;
    drawCount = 0;
}

// Constructs a wall with the given parameters
void Building::AddFace(Orientation dir, TexOffset type, int variant, int story, int blocks)
{
    bool doorPlaced = false;

    // Calculate texture offsets for upper left vertices of the wall faces
    float texOffsets[blocks * 2];
    for (int i = 0; i < blocks * 2; i += 2)
    {
        if (type == TexOffset::Door)
        {
            if (!doorPlaced && (boolDist(rng) || i == blocks * 2 - 2))
            {
                texOffsets[i] = (float)type * tileSizeNormalized.x;
                texOffsets[i + 1] = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;
                doorPlaced = true;
            }
            else
            {
                texOffsets[i] = (float)RandomWallType() * tileSizeNormalized.x;
                texOffsets[i + 1] = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;
            }
        }
        else
        {
            if (type == TexOffset::Wall || type == TexOffset::Window || type == TexOffset::LargeWindow)
            {
                texOffsets[i] = (float)RandomWallType() * tileSizeNormalized.x;
                texOffsets[i + 1] = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;
            }
            else
            {
                texOffsets[i] = (float)type * tileSizeNormalized.x;
                texOffsets[i + 1] = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;
            }
        }
    }

    // Calculate story dimensions and offsets
    float storySize = 1.0f;
    float halfSize = 0.5f;
    float yPosOffs = storySize * story;
    float xOffset = -halfSize * (blocks - 1);
    float zOffset = xOffset;

    switch (dir)
    {
        // Construct a global north face (Z-)
        case Orientation::North:
            for (int i = 0; i < blocks; i++)
            {
                mesh.AddQuad(
                    {halfSize + xOffset + pos.x, yPosOffs + storySize + pos.y, -halfSize * blocks + pos.z, 0.0f, 0.0f, -1.0f, texOffsets[i * 2], texOffsets[i * 2 + 1]},
                    {-halfSize + xOffset + pos.x, yPosOffs + storySize + pos.y, -halfSize * blocks + pos.z, 0.0f, 0.0f, -1.0f, texOffsets[i * 2] + tileSizeNormalized.x, texOffsets[i * 2 + 1]},
                    {halfSize + xOffset + pos.x, yPosOffs + pos.y, -halfSize * blocks + pos.z, 0.0f, 0.0f, -1.0f, texOffsets[i * 2], texOffsets[i * 2 + 1] - tileSizeNormalized.y},
                    {-halfSize + xOffset + pos.x, yPosOffs + pos.y, -halfSize * blocks + pos.z, 0.0f, 0.0f, -1.0f, texOffsets[i * 2] + tileSizeNormalized.x, texOffsets[i * 2 + 1] - tileSizeNormalized.y}
                );

                // Adjust offset
                xOffset += storySize;
            }
            break;
        
        // Construct a global east face (X+)
        case Orientation::East:
            for (int i = 0; i < blocks; i++)
            {
                mesh.AddQuad(
                    {halfSize * blocks + pos.x, yPosOffs + storySize + pos.y, halfSize + xOffset + pos.z, 1.0f, 0.0f, 0.0f, texOffsets[i * 2], texOffsets[i * 2 + 1]},
                    {halfSize * blocks + pos.x, yPosOffs + storySize + pos.y, -halfSize + xOffset + pos.z, 1.0f, 0.0f, 0.0f, texOffsets[i * 2] + tileSizeNormalized.x, texOffsets[i * 2 + 1]},
                    {halfSize * blocks + pos.x, yPosOffs + pos.y, halfSize + xOffset + pos.z, 1.0f, 0.0f, 0.0f, texOffsets[i * 2], texOffsets[i * 2 + 1] - tileSizeNormalized.y},
                    {halfSize * blocks + pos.x, yPosOffs + pos.y, -halfSize + xOffset + pos.z, 1.0f, 0.0f, 0.0f, texOffsets[i * 2] + tileSizeNormalized.x, texOffsets[i * 2 + 1] - tileSizeNormalized.y}
                );

                // Adjust offset
                xOffset += storySize;
            }
            break;
        
        // Construct a global south face (Z+)
        case Orientation::South:
            for (int i = 0; i < blocks; i++)
            {
                mesh.AddQuad(
                    {-halfSize + xOffset + pos.x, yPosOffs + storySize + pos.y, halfSize * blocks + pos.z, 0.0f, 0.0f, 1.0f, texOffsets[i * 2], texOffsets[i * 2 + 1]},
                    {halfSize + xOffset + pos.x, yPosOffs + storySize + pos.y, halfSize * blocks + pos.z, 0.0f, 0.0f, 1.0f, texOffsets[i * 2] + tileSizeNormalized.x, texOffsets[i * 2 + 1]},
                    {-halfSize + xOffset + pos.x, yPosOffs + pos.y, halfSize * blocks + pos.z, 0.0f, 0.0f, 1.0f, texOffsets[i * 2], texOffsets[i * 2 + 1] - tileSizeNormalized.y},
                    {halfSize + xOffset + pos.x, yPosOffs + pos.y, halfSize * blocks + pos.z, 0.0f, 0.0f, 1.0f, texOffsets[i * 2] + tileSizeNormalized.x, texOffsets[i * 2 + 1] - tileSizeNormalized.y}
                );

                // Adjust offset
                xOffset += storySize;
            }
            
            break;
        
        // Construct a global west face (X-)
        case Orientation::West:
            for (int i = 0; i < blocks; i++)
            {
                mesh.AddQuad(
                    {-halfSize * blocks + pos.x, yPosOffs + storySize + pos.y, -halfSize + xOffset + pos.z, -1.0f, 0.0f, 0.0f, texOffsets[i * 2], texOffsets[i * 2 + 1]},
                    {-halfSize * blocks + pos.x, yPosOffs + storySize + pos.y, halfSize + xOffset + pos.z, -1.0f, 0.0f, 0.0f, texOffsets[i * 2] + tileSizeNormalized.x, texOffsets[i * 2 + 1]},
                    {-halfSize * blocks + pos.x, yPosOffs + pos.y, -halfSize + xOffset + pos.z, -1.0f, 0.0f, 0.0f, texOffsets[i * 2], texOffsets[i * 2 + 1] - tileSizeNormalized.y},
                    {-halfSize * blocks + pos.x, yPosOffs + pos.y, halfSize + xOffset + pos.z, -1.0f, 0.0f, 0.0f, texOffsets[i * 2] + tileSizeNormalized.x, texOffsets[i * 2 + 1] - tileSizeNormalized.y}
                );

                // Adjust offset
                xOffset += storySize;
            }
            break;

        // Construct a global up face (Y+)
        case Orientation::Up:
            for (int i = 0; i < blocks * blocks; i++)
            {
                mesh.AddQuad(
                    {-halfSize + xOffset + pos.x, yPosOffs + storySize + pos.y, -halfSize + zOffset + pos.z, 0.0f, 1.0f, 0.0f, texOffsets[0], texOffsets[1]},
                    {halfSize + xOffset + pos.x, yPosOffs + storySize + pos.y, -halfSize + zOffset + pos.z, 0.0f, 1.0f, 0.0f, texOffsets[0] + tileSizeNormalized.x, texOffsets[1]},
                    {-halfSize + xOffset + pos.x, yPosOffs + storySize + pos.y, halfSize + zOffset + pos.z, 0.0f, 1.0f, 0.0f, texOffsets[0], texOffsets[1] - tileSizeNormalized.y},
                    {halfSize + xOffset + pos.x, yPosOffs + storySize + pos.y, halfSize + zOffset + pos.z, 0.0f, 1.0f, 0.0f, texOffsets[0] + tileSizeNormalized.x, texOffsets[1] - tileSizeNormalized.y}
                );

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