#pragma once

#include <vector>
#include <glm/glm.hpp>

class Building
{
    // Feature flags for generating extra index data
    enum class FeatureFlags : int
    {
        Awning,
        Deck,
        Chimney
    };

    // Interface
    public:
        Building(const glm::ivec3& pos, int stories, int width, int height, int variant, FeatureFlags features);
        ~Building();

        // Delete copy constructor/assignment
        Building(const Building&) = delete;
        Building& operator=(const Building&) = delete;

        // Delete move constructor/assignment
        Building(Building&& other) = delete;
        void operator=(Building&& other) = delete;

        // Constants
        static const int MAX_STORIES = 16;
        static const int NUM_VARIANTS = 4;

        // Rendering methods
        void Draw();
    
    // Data / implementation
    private:
        
};