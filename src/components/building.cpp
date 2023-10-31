#include "building.hpp"

// Main constructor
Building::Building(const glm::ivec3& pos, int stories, float storySize, int variant, FeatureFlags features, Orientation orientation)
{
    // Initialize static resources if first instance
    if (refCount == 0)
    {
        texture = new Texture2D("data/buildingAtlas.png");
        vbo = new GPUBuffer(BufferType::DynamicVertex, sizeof(VertexPosNormUv) * MAX_VERTICES);
        vao = new VertexAttributes(VertexFormat::POS_NORM_UV, vbo);
    }
    refCount++;

    // Clamp to safe input values
    stories = std::clamp(stories, 1, MAX_STORIES);
    variant = std::clamp(variant, 0, NUM_VARIANTS);

    // Only need to calculate these once
    float halfSize = 0.5f * storySize;
    float w = texture->GetWidth();
    float h = texture->GetHeight();
    float yOffset = 0;

    // Initialize texture atlas offsets
    glm::vec2 texOffset = {0, variant * (h / NUM_VARIANTS)};

    // Generate the first story

    // Decide texOffset based on orientation
    if (orientation == Orientation::North)
        texOffset.x = (int)TexOffset::Door * w;
    else
        texOffset.x = (int)TexOffset::Side * w;

    // North wall
    vertices.push_back({halfSize, storySize, halfSize,   0.0f, 0.0f, 1.0f,   texOffset.x, texOffset.y});
    
    // Generate each additional story's vertex data
    for (int i = 0; i < stories; i++)
    {
        // Calculate story offset
        yOffset += storySize;
    }

    // Generate the final story + roof
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
        delete vao;
    }
}