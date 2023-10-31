#pragma once

#include <algorithm>
#include <vector>
#include <glm/glm.hpp>

#include "../resources/gpubuffer.hpp"
#include "../resources/texture2d.hpp"
#include "../resources/vertex.hpp"
#include "../resources/vertexattributes.hpp"

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

    // Building texture atlas offsets
    enum class TexOffset : int
    {
        Door = 0,
        Side,
        Window,
        LargeWindow,
        Roof,
        Awning
    };

    // Interface
    public:
        Building(const glm::ivec3& pos, int stories, float storySize, int variant, FeatureFlags features, Orientation orientation);
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
        static const inline int MAX_VERTICES = 16'384;
        static const inline int TILE_SIZE = 64;

        // Rendering methods
        void Draw();
    
    // Data / implementation
    private:
        // Instance vertices
        std::vector<VertexPosNormUv> vertices;

        // Static resources
        static inline Texture2D* texture = nullptr;
        static inline GPUBuffer* vbo = nullptr;
        static inline VertexAttributes* vao = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;
};