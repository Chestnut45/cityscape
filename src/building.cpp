#include "building.hpp"

// Main constructor
Building::Building(const glm::vec3 &pos, int stories, int baseBlockCount, int variant, Orientation orientation)
{
    // Initialize static resources if first instance
    if (refCount == 0)
    {
        // Load the texture atlas
        textureAtlas = new Phi::Texture2D("data/textures/buildingAtlas.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, true);

        // Compile the shader
        shader = new Phi::Shader();
        shader->LoadShaderSource(GL_VERTEX_SHADER, "data/shaders/building.vs");
        shader->LoadShaderSource(GL_FRAGMENT_SHADER, "data/shaders/building.fs");
        shader->Link();

        // Initialize the render batch
        renderBatch = new Phi::RenderBatch<Phi::VertexPosNormUv>(65'536, 131'072);

        // Calculate normalized tile size for atlas offsets
        int w = textureAtlas->GetWidth();
        int h = textureAtlas->GetHeight();
        tileSize = w / (int)TexOffset::Count;
        tileSizeNormalized = glm::vec2((float)tileSize / w, (float)tileSize / h);
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
}

// Cleanup
Building::~Building()
{
    refCount--;
    if (refCount == 0)
    {
        // Cleanup static resources
        delete textureAtlas;
        delete shader;
        delete renderBatch;
    }
}

// Draws this building into the static buffer
void Building::Draw() const
{
    // TODO: Reactive flushing could be made simpler
    if (!renderBatch->AddMesh(mesh))
    {
        FlushDrawCalls();
        renderBatch->AddMesh(mesh);
    }
}

// Flushes all buildings drawn since last flush
void Building::FlushDrawCalls()
{
    // Bind relevant resources
    textureAtlas->Bind();

    // Issue rendering commands
    renderBatch->Flush(*shader);
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