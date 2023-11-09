#pragma once

#include <glm/glm.hpp>

#include "../resources/gpubuffer.hpp"
#include "../resources/vertexattributes.hpp"
#include "../resources/vertex.hpp"
#include "../resources/shader.hpp"

struct DirectionalLight
{
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 color;
};

class PointLight
{
    // Interface
    public:
        PointLight();
        ~PointLight();

        // Delete copy constructor/assignment
        PointLight(const PointLight&) = delete;
        PointLight& operator=(const PointLight&) = delete;

        // Delete move constructor/assignment
        PointLight(PointLight&& other) = delete;
        void operator=(PointLight&& other) = delete;

        // Draws into instance buffer, flushing if it is full
        void Draw();

        // Flushes all grounds drawn since the last flush
        static void FlushDrawCalls();
    
    // Data / implementation
    private:
        glm::vec4 position;
        glm::vec4 color;

        // Instancing information
        static const int MAX_INSTANCES = 128;
        static inline int drawCount = 0;

        // Static resources
        static inline GPUBuffer* vbo = nullptr;
        static inline GPUBuffer* ebo = nullptr;
        static inline GPUBuffer* instanceUBO = nullptr;
        static inline VertexAttributes* vao = nullptr;
        static inline Shader* shader = nullptr;
};

// Wrapped it in a namespace so globals X and Z are constrained to this scope
namespace Icosphere
{
    // Icosphere data
    static const float X = 0.525731112119133606f;
    static const float Z = 0.850650808352039932f;

    static const VertexPos ICOSPHERE_VERTICES[] =
    {
        {-X, 0.0f, Z}, {X, 0.0f, Z}, {-X, 0.0f, -Z}, {X, 0.0f, -Z},
        {0.0f, Z, X}, {0.0f, Z, -X}, {0.0f, -Z, X}, {0.0f, -Z, -X},
        {Z, X, 0.0f}, {-Z, X, 0.0f}, {Z, -X, 0.0f}, {-Z, -X, 0.0f}
    };

    static const GLuint ICOSPHERE_INDICES[] =
    {
        0, 4, 1,
        0, 9, 4,
        9, 5, 4,
        4, 5, 8,
        4, 8, 1,
        8, 10, 1,
        8, 3, 10,
        5, 3, 8,
        5, 2, 3,
        2, 7, 3,
        7, 10, 3,
        7, 6, 10,
        7, 11, 6,
        11, 0, 6,
        0, 1, 6,
        6, 1, 10,
        9, 0, 11,
        9, 11, 2,
        9, 2, 5,
        7, 2, 11
    };
}