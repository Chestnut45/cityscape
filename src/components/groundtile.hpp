#pragma once

#include <glm/glm.hpp>

#include "../phi/gpubuffer.hpp"
#include "../phi/shader.hpp"
#include "../phi/texture2d.hpp"
#include "../phi/vertexattributes.hpp"
#include "../phi/vertex.hpp"
#include "../phi/mesh.hpp"

class GroundTile
{
    // Interface
    public:
        GroundTile(const glm::ivec2& id);
        ~GroundTile();

        // Delete copy constructor/assignment
        GroundTile(const GroundTile&) = delete;
        GroundTile& operator=(const GroundTile&) = delete;

        // Delete move constructor/assignment
        GroundTile(GroundTile&& other) = delete;
        void operator=(GroundTile&& other) = delete;

        // Draws into instance buffer, flushing if it is full
        void Draw();

        // Flushes all grounds drawn since the last flush
        static void FlushDrawCalls();

        // Accessors
        inline const glm::vec4& GetPosition() const { return position; };

    // Data / Implementation
    private:
        // State
        glm::vec4 position;

        // Instancing information
        static const int MAX_INSTANCES = 128;
        static inline int drawCount = 0;

        // Static resources
        static inline Texture2D* texture = nullptr;
        static inline GPUBuffer* vbo = nullptr;
        static inline GPUBuffer* ebo = nullptr;
        static inline VertexAttributes* vao = nullptr;
        static inline GPUBuffer* instanceUBO = nullptr;
        static inline Shader* shader = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;
};