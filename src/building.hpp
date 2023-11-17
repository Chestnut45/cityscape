#pragma once

#include <algorithm>
#include <vector>
#include <random>

#include <glm/glm.hpp>

#include <phi/gpubuffer.hpp>
#include <phi/shader.hpp>
#include <phi/texture2d.hpp>
#include <phi/vertex.hpp>
#include <phi/vertexattributes.hpp>

class Building
{
    // Interface
    public:
        // Feature flags for generating extra index data
        enum class Feature : int
        {
            None,
            Awning,
            Deck,
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
            Awning
        };

        // Constructor
        Building(const glm::ivec3& pos, int stories, int blocks, int variant, Feature features = Feature::None, Orientation orientation = Orientation::North);
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
        static const inline int MAX_VERTICES = 32'768;
        static const inline int MAX_INDICES = 131'072;

        // Rendering methods
        void Draw();
        static void FlushDrawCalls();
    
    // Data / implementation
    private:
        // Instance vertex / index data
        std::vector<VertexPosNormUv> vertices;
        std::vector<GLuint> indices;

        // Offset indices, used for batching purposes
        std::vector<GLuint> offsetIndices;

        // Sizes and counters
        size_t vertBytes = 0;
        size_t indBytes = 0;
        static inline GLuint vertexCount = 0;
        static inline GLuint indexCount = 0;
        static inline GLuint drawCount = 0;

        // Tile sizes
        static const inline int TILE_SIZE = 64;
        static inline glm::vec2 tileSizeNormalized;

        // Static resources
        static inline Texture2D* textureAtlas = nullptr;
        static inline GPUBuffer* vbo = nullptr;
        static inline GPUBuffer* ebo = nullptr;
        static inline VertexAttributes* vao = nullptr;
        static inline Shader* shader = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;

        // Helper methods for procedural generation
        void AddFace(Orientation dir, TexOffset type, int variant, int story, int blocks);
        void AddFeature(Feature feature, Orientation orientation);
        TexOffset RandomWallType() const;

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