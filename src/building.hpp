#pragma once

#include <algorithm>
#include <vector>
#include <random>

#include <glm/glm.hpp>

#include <phi/phi.hpp>

class Building
{
    // Interface
    public:

        // Feature flags for generating extra vertex data
        enum class Feature : int
        {
            Awning,
            Balcony,
            Chimney
        };

        // Valid building face directions
        enum class Orientation : int
        {
            North,
            East,
            South,
            West,
            Up,
            Down // unused but here for completeness
        };

        // Building texture atlas offsets
        enum class TexOffset : int
        {
            Door = 0,
            Wall,
            Window,
            LargeWindow,
            Roof,
            Awning,

            // Number of columns in the atlas, for calculating tile size (leave at end)
            Count
        };

        // Constructor
        Building(const glm::vec3& pos, int stories, int baseBlockCount, int variant, Orientation orientation = Orientation::North);
        ~Building();

        // Delete copy constructor/assignment
        Building(const Building&) = delete;
        Building& operator=(const Building&) = delete;

        // Delete move constructor/assignment
        Building(Building&& other) = delete;
        void operator=(Building&& other) = delete;

        // Constants
        static const inline int MAX_STORIES = 16;
        static const inline int NUM_VARIANTS = 4;

        // Rendering methods
        void Draw() const;
        static void FlushDrawCalls();
    
    // Data / implementation
    private:

        // World position of building
        glm::vec3 pos;

        // Mesh instance to be procedurally generated
        Phi::Mesh<Phi::VertexPosNormUv> mesh;

        // Helper methods for procedural generation
        void AddFace(Orientation dir, TexOffset type, int variant, int story, int blocks);
        void AddFeature(Feature feature, Orientation orientation);
        TexOffset RandomWallType() const;

        // Tile sizes
        static inline int tileSize;
        static inline glm::vec2 tileSizeNormalized;

        // Static resources
        static inline Phi::Texture2D* textureAtlas = nullptr;
        static inline Phi::Shader* shader = nullptr;
        static inline Phi::RenderBatch<Phi::VertexPosNormUv>* renderBatch = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;

        // RNG
        static inline std::default_random_engine rng;
        static inline std::uniform_int_distribution<int> wallDist{(int)TexOffset::Wall, (int)TexOffset::LargeWindow};
        static inline std::uniform_int_distribution<int> stepDist{0, 6};
        static inline std::uniform_int_distribution<int> boolDist{0, 1};
};

// Offset locations relative to a city block origin for buildings
static const glm::vec3 smallBuildingOffsets[] =
{
    {4, 0, 7},
    {4, 0, 4},
    {7, 0, 4},
    {9, 0, 4},
    {12, 0, 4},
    {12, 0, 7},
    {12, 0, 9},
    {12, 0, 12},
    {9, 0, 12},
    {7, 0, 12},
    {4, 0, 12},
    {4, 0, 9},
};

static const Building::Orientation smallBuildingOrientations[] =
{
    Building::Orientation::West,
    Building::Orientation::North,
    Building::Orientation::North,
    Building::Orientation::North,
    Building::Orientation::East,
    Building::Orientation::East,
    Building::Orientation::East,
    Building::Orientation::South,
    Building::Orientation::South,
    Building::Orientation::South,
    Building::Orientation::West,
    Building::Orientation::West
};

static const glm::vec3 largeBuildingOffsets[] =
{
    {5, 0, 5},
    {11, 0, 5},
    {11, 0, 11},
    {5, 0, 11}
};

static const Building::Orientation largeBuildingOrientations[] =
{
    Building::Orientation::West,
    Building::Orientation::North,
    Building::Orientation::East,
    Building::Orientation::South
};