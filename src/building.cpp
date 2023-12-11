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
        AddFace(Orientation::North, RandomWallType(i), variant, i, currentStoryBlocks);
        AddFace(Orientation::East, RandomWallType(i), variant, i, currentStoryBlocks);
        AddFace(Orientation::South, RandomWallType(i), variant, i, currentStoryBlocks);
        AddFace(Orientation::West, RandomWallType(i), variant, i, currentStoryBlocks);

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

    // Calculate texture offsets
    float texOffsets[blocks * 2];
    for (int i = 0; i < blocks * 2; i += 2)
    {
        switch (type)
        {
            case TexOffset::Door:

                // Ensure we eventually place a door if rng doesn't first
                if (!doorPlaced && (boolDist(rng) || i == blocks * 2 - 2))
                {
                    texOffsets[i] = (float)type * tileSizeNormalized.x;
                    texOffsets[i + 1] = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;
                    doorPlaced = true;
                }
                else
                {
                    texOffsets[i] = (float)RandomWallType(story) * tileSizeNormalized.x;
                    texOffsets[i + 1] = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;
                }

                break;
            
            default:

                texOffsets[i] = (float)type * tileSizeNormalized.x;
                texOffsets[i + 1] = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y;

                break;
        }
    }

    // Calculate story dimensions and offsets
    float storySize = 1.0f;
    float halfSize = 0.5f;
    float yPosOffs = storySize * story;
    float xOffset = -halfSize * (blocks - 1);
    float zOffset = xOffset;
    bool keepGenFeatures = true;

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

                if (boolDist(rng)) keepGenFeatures = keepGenFeatures ? AddFeature(type, dir, {xOffset + pos.x, yPosOffs + halfSize + pos.y, -halfSize * blocks + pos.z}, variant, story, blocks) : keepGenFeatures;

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

                if (boolDist(rng)) keepGenFeatures = keepGenFeatures ? AddFeature(type, dir, {halfSize * blocks + pos.x, yPosOffs + halfSize + pos.y, xOffset + pos.z}, variant, story, blocks) : keepGenFeatures;

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

                if (boolDist(rng)) keepGenFeatures = keepGenFeatures ? AddFeature(type, dir, {xOffset + pos.x, yPosOffs + halfSize + pos.y, halfSize * blocks + pos.z}, variant, story, blocks) : keepGenFeatures;

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

                if (boolDist(rng)) keepGenFeatures = keepGenFeatures ? AddFeature(type, dir, {-halfSize * blocks + pos.x, yPosOffs + halfSize + pos.y, xOffset + pos.z}, variant, story, blocks) : keepGenFeatures;

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

// Adds a feature to the building, returning whether or not to keep generating features
// for that specific face
bool Building::AddFeature(TexOffset type, Orientation orientation, const glm::vec3& facePos, int variant, int story, int blocks)
{
    // Build rotation matrix based on orientation
    glm::mat4 rotation = glm::mat4(1.0f);
    switch (orientation)
    {
        case Orientation::North: break;
        case Orientation::East: rotation = glm::rotate(rotation, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); break;
        case Orientation::South: rotation = glm::rotate(rotation, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); break;
        case Orientation::West: rotation = glm::rotate(rotation, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); break;
        default: break;
    }

    // TODO: Actual texture coordinates
    float variantOffset = (float)(NUM_VARIANTS - variant) * tileSizeNormalized.y - 0.001f;

    switch (type)
    {
        case TexOffset::Door:
        {
            // Generate awning

            // Vertex positions
            glm::vec4 posA = rotation * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f) + glm::vec4(facePos, 1.0f);
            glm::vec4 posB = rotation * glm::vec4(-0.5f, 0.0f, -0.5f, 1.0f) + glm::vec4(facePos, 1.0f);
            glm::vec4 posC = rotation * glm::vec4(-0.5f, 0.0f, 0.0f, 1.0f) + glm::vec4(facePos, 1.0f);

            glm::vec4 posD = rotation * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f) + glm::vec4(facePos, 1.0f);
            glm::vec4 posE = rotation * glm::vec4(0.5f, 0.0f, -0.5f, 1.0f) + glm::vec4(facePos, 1.0f);
            glm::vec4 posF = rotation * glm::vec4(0.5f, 0.0f, 0.0f, 1.0f) + glm::vec4(facePos, 1.0f);

            // Normals
            glm::vec4 n1 = rotation * glm::vec4(-1.0f, 0.0f, 0.0f, 1.0f);
            glm::vec4 n2 = rotation * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            glm::vec4 n3 = rotation * glm::vec4(0.0f, 0.5f, -0.5f, 1.0f);
            glm::vec4 n4 = rotation * glm::vec4(0.0f, -0.5f, 0.5f, 1.0f);
            

            // Side 1
            mesh.AddTriangle(
                {posA.x, posA.y, posA.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
                {posB.x, posB.y, posB.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
                {posC.x, posC.y, posC.z, n1.x, n1.y, n1.z, 0.0f, variantOffset}
            );
            mesh.AddTriangle(
                {posC.x, posC.y, posC.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
                {posB.x, posB.y, posB.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
                {posA.x, posA.y, posA.z, n2.x, n2.y, n2.z, 0.0f, variantOffset}
            );

            // Side 2
            mesh.AddTriangle(
                {posD.x, posD.y, posD.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
                {posE.x, posE.y, posE.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
                {posF.x, posF.y, posF.z, n1.x, n1.y, n1.z, 0.0f, variantOffset}
            );
            mesh.AddTriangle(
                {posF.x, posF.y, posF.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
                {posE.x, posE.y, posE.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
                {posD.x, posD.y, posD.z, n2.x, n2.y, n2.z, 0.0f, variantOffset}
            );

            // Top
            mesh.AddQuad(
                {posA.x, posA.y, posA.z, n3.x, n3.y, n3.z, 0.0f, variantOffset},
                {posB.x, posB.y, posB.z, n3.x, n3.y, n3.z, 0.0f, variantOffset},
                {posD.x, posD.y, posD.z, n3.x, n3.y, n3.z, 0.0f, variantOffset},
                {posE.x, posE.y, posE.z, n3.x, n3.y, n3.z, 0.0f, variantOffset}
            );
            mesh.AddQuad(
                {posA.x, posA.y, posA.z, n4.x, n4.y, n4.z, 0.0f, variantOffset},
                {posD.x, posD.y, posD.z, n4.x, n4.y, n4.z, 0.0f, variantOffset},
                {posB.x, posB.y, posB.z, n4.x, n4.y, n4.z, 0.0f, variantOffset},
                {posE.x, posE.y, posE.z, n4.x, n4.y, n4.z, 0.0f, variantOffset}
            );

            return false;
            break;
        }
        
        case TexOffset::Wall:
        case TexOffset::Window:
        case TexOffset::LargeWindow:

            // Generate balcony
            // if (story != 0 && boolDist(rng) == 0 && blocks > 1)
            // {
            //     // Vertex positions
            //     glm::vec4 posA = rotation * glm::vec4(-0.5f, 0.0f, 0.0f, 1.0f) + glm::vec4(facePos, 1.0f);
            //     glm::vec4 posB = rotation * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f) + glm::vec4(facePos, 1.0f);
            //     glm::vec4 posC = rotation * glm::vec4(-0.5f, 0.0f, -0.5f, 1.0f) + glm::vec4(facePos, 1.0f);
            //     glm::vec4 posD = rotation * glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f) + glm::vec4(facePos, 1.0f);

            //     glm::vec4 posE = rotation * glm::vec4(0.5f, 0.0f, 0.0f, 1.0f) + glm::vec4(facePos, 1.0f);
            //     glm::vec4 posF = rotation * glm::vec4(0.5f, -0.5f, 0.0f, 1.0f) + glm::vec4(facePos, 1.0f);
            //     glm::vec4 posG = rotation * glm::vec4(0.5f, 0.0f, -0.5f, 1.0f) + glm::vec4(facePos, 1.0f);
            //     glm::vec4 posH = rotation * glm::vec4(0.5f, -0.5f, -0.5f, 1.0f) + glm::vec4(facePos, 1.0f);

            //     // Normals
            //     glm::vec4 n1 = rotation * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            //     glm::vec4 n2 = rotation * glm::vec4(-1.0f, 0.0f, 0.0f, 1.0f);
            //     glm::vec4 n3 = rotation * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            //     glm::vec4 n4 = rotation * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);

            //     // Side 1
            //     mesh.AddQuad(
            //         {posA.x, posA.y, posA.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
            //         {posC.x, posC.y, posC.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
            //         {posB.x, posB.y, posB.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
            //         {posD.x, posD.y, posD.z, n1.x, n1.y, n1.z, 0.0f, variantOffset}
            //     );
            //     mesh.AddQuad(
            //         {posA.x, posA.y, posA.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
            //         {posB.x, posB.y, posB.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
            //         {posC.x, posC.y, posC.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
            //         {posD.x, posD.y, posD.z, n2.x, n2.y, n2.z, 0.0f, variantOffset}
            //     );

            //     // Side 2
            //     mesh.AddQuad(
            //         {posC.x, posC.y, posC.z, n3.x, n3.y, n3.z, 0.0f, variantOffset},
            //         {posG.x, posG.y, posG.z, n3.x, n3.y, n3.z, 0.0f, variantOffset},
            //         {posD.x, posD.y, posD.z, n3.x, n3.y, n3.z, 0.0f, variantOffset},
            //         {posH.x, posH.y, posH.z, n3.x, n3.y, n3.z, 0.0f, variantOffset}
            //     );
            //     mesh.AddQuad(
            //         {posC.x, posC.y, posC.z, n4.x, n4.y, n4.z, 0.0f, variantOffset},
            //         {posD.x, posD.y, posD.z, n4.x, n4.y, n4.z, 0.0f, variantOffset},
            //         {posG.x, posG.y, posG.z, n4.x, n4.y, n4.z, 0.0f, variantOffset},
            //         {posH.x, posH.y, posH.z, n4.x, n4.y, n4.z, 0.0f, variantOffset}
            //     );

            //     // Side 3
            //     mesh.AddQuad(
            //         {posG.x, posG.y, posG.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
            //         {posE.x, posE.y, posE.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
            //         {posH.x, posH.y, posH.z, n2.x, n2.y, n2.z, 0.0f, variantOffset},
            //         {posF.x, posF.y, posF.z, n2.x, n2.y, n2.z, 0.0f, variantOffset}
            //     );
            //     mesh.AddQuad(
            //         {posG.x, posG.y, posG.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
            //         {posH.x, posH.y, posH.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
            //         {posE.x, posE.y, posE.z, n1.x, n1.y, n1.z, 0.0f, variantOffset},
            //         {posF.x, posF.y, posF.z, n1.x, n1.y, n1.z, 0.0f, variantOffset}
            //     );
            // }

            return false;
            break;
        
        default:
            return false;
            break;
    }
}

// Randomly chooses a wall type (not seeded)
Building::TexOffset Building::RandomWallType(int story) const
{
    if (story == 0)
    {
        return boolDist(rng) ? TexOffset::Window : TexOffset::LargeWindow;
    }
    return (TexOffset)wallDist(rng);
}