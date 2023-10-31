#include "building.hpp"

// Main constructor
Building::Building(const glm::ivec3& pos, int stories, int width, int depth, int variant, FeatureFlags features, Orientation orientation)
{
    // Clamp to safe input values
    stories = std::clamp(stories, 1, MAX_STORIES);
    
    for (int i = 0; i < stories; i++)
    {
        // Generate each story's vertex data
        
    }
}

// Cleanup
Building::~Building()
{
    
}