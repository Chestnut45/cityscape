#pragma once

#include <algorithm>
#include <vector>
#include <glm/glm.hpp>

#include "../resources/vertex.hpp"

class Building
{
    // Feature flags for generating extra index data
    enum class FeatureFlags : int
    {
        Awning,
        Deck,
        Chimney
    };

    // Valid building facing directions
    enum class Orientation : int
    {
        North,
        East,
        South,
        West
    };

    // Interface
    public:
        Building(const glm::ivec3& pos, int stories, int width, int depth, int variant, FeatureFlags features, Orientation orientation);
        ~Building();

        // Delete copy constructor/assignment
        Building(const Building&) = delete;
        Building& operator=(const Building&) = delete;

        // Delete move constructor/assignment
        Building(Building&& other) = delete;
        void operator=(Building&& other) = delete;

        // Constants
        static const inline int MAX_STORIES = 16;
        static const inline int NUM_VARIANTS = 2;

        // Rendering methods
        void Draw();
    
    // Data / implementation
    private:
        
};