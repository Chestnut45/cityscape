#pragma once

#include <glm/glm.hpp>

#include <phi/phi.hpp>

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
        static inline Phi::Texture2D* texture = nullptr;
        static inline Phi::GPUBuffer* vbo = nullptr;
        static inline Phi::GPUBuffer* ebo = nullptr;
        static inline Phi::VertexAttributes* vao = nullptr;
        static inline Phi::GPUBuffer* instanceUBO = nullptr;
        static inline Phi::Shader* shader = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;
};