#pragma once

#include <glm/glm.hpp>

#include <phi/phi.hpp>

class DirectionalLight
{
    // Interface
    public:
        DirectionalLight(const glm::vec4& pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
                        const glm::vec4& dir = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                        const glm::vec4& col = glm::vec4(1.0f));
        ~DirectionalLight();

        // Delete copy constructor/assignment
        DirectionalLight(const DirectionalLight&) = delete;
        DirectionalLight& operator=(const DirectionalLight&) = delete;

        // Delete move constructor/assignment
        DirectionalLight(DirectionalLight&& other) = delete;
        void operator=(DirectionalLight&& other) = delete;

        // Mutators
        inline void SetPosition(const glm::vec4& pos) { position = pos; };
        inline void SetDirection(const glm::vec4& dir) { direction = dir; };
        inline void SetColor(const glm::vec4& col) { color = col; };
        inline void TurnOn() { on = true; };
        inline void TurnOff() { on = false; };
        inline bool IsOn() const { return on; };

        // Accessors
        inline const glm::vec4& GetPosition() const { return position; };
        inline const glm::vec4& GetDirection() const { return direction; };
        inline const glm::vec4& GetColor() const { return color; };

    // Data / implementation
    private:
        // Per-instance data
        glm::vec4 position;
        glm::vec4 direction;
        glm::vec4 color;
        bool on = true;
};

class PointLight
{
    // Interface
    public:
        PointLight(const glm::vec4& pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), const glm::vec4& col = glm::vec4(1.0f));
        ~PointLight();

        // Delete copy constructor/assignment
        PointLight(const PointLight&) = delete;
        PointLight& operator=(const PointLight&) = delete;

        // Delete move constructor/assignment
        PointLight(PointLight&& other) = delete;
        void operator=(PointLight&& other) = delete;

        // Mutators
        inline void SetPosition(const glm::vec4& pos) { position = pos; };
        inline void SetColor(const glm::vec4& col) { color = col; };
        inline void TurnOn() { on = true; };
        inline void TurnOff() { on = false; };
        inline bool IsOn() const { return on; };

        // Accessors
        inline const glm::vec4& GetPosition() const { return position; };
        inline const glm::vec4& GetColor() const { return color; };

        // Draws into instance buffer, flushing if it is full
        void Draw();

        // Flushes all grounds drawn since the last flush
        static void FlushDrawCalls();
    
    // Data / implementation
    private:
        // Per-Instance data
        glm::vec4 position;
        glm::vec4 color;
        bool on = true;

        // Instancing information
        static const int MAX_INSTANCES = 512;
        static inline int drawCount = 0;

        // Static resources
        static inline Phi::GPUBuffer* vbo = nullptr;
        static inline Phi::GPUBuffer* ebo = nullptr;
        static inline Phi::VertexAttributes* vao = nullptr;

        static inline Phi::GPUBuffer* instanceUBO = nullptr;
        static inline Phi::Shader* shader = nullptr;

        // Reference counting for static resources
        static inline int refCount = 0;
};