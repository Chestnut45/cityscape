#pragma once

#include <glm/glm.hpp>

#include "../resources/texture2d.hpp"
#include "../resources/gpubuffer.hpp"
#include "../resources/vertexattributes.hpp"
#include "../resources/vertex.hpp"

class GroundTile
{
    // Interface
    public:
        GroundTile(const glm::vec3& pos);
        ~GroundTile();

        // Delete copy constructor/assignment
        GroundTile(const GroundTile&) = delete;
        GroundTile& operator=(const GroundTile&) = delete;

        // Delete move constructor/assignment
        GroundTile(GroundTile&& other) = delete;
        void operator=(GroundTile&& other) = delete;

        // Temporal manipulation
        inline void SetPosition(float x, float y, float z) { position = {x, y, z}; };

        // Draws into instance buffer, flushing if it is full
        void Draw();

        // Flushes all grounds drawn since the last flush
        static void FlushDrawCalls();

    // Data / Implementation
    private:
        // State
        glm::vec3 position;

        // Instance data
        static const int MAX_INSTANCES = 128;
        static inline int drawCount = 0;

        // Static resources
        static inline Texture2D* texture = nullptr;
        static inline GPUBuffer* vbo = nullptr;
        static inline GPUBuffer* ebo = nullptr;
        static inline VertexAttributes* vao = nullptr;
        static inline GPUBuffer* instanceUBO = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;
};