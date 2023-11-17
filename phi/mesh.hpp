#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "gpubuffer.hpp"
#include "texture2d.hpp"
#include "vertex.hpp"
#include "vertexattributes.hpp"
#include "shader.hpp"

// Represents a renderable mesh of arbitrary format
// NOTE: Vertex and InstanceData are assumed to be GPUBuffer-writable formats
template <typename Vertex, typename InstanceData = glm::mat4>
class Mesh
{
    // Interface
    public:

        Mesh();
        ~Mesh();

        // Delete copy constructor/assignment
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        // Delete move constructor/assignment
        Mesh(Mesh&& other) = delete;
        void operator=(Mesh&& other) = delete;

        // Procedural vertex data generation
        void AddSurface(const std::vector<Vertex>& vertices, const std::vector<GLuint>* const indices = nullptr);
        void AddTriangle(const Vertex& a, const Vertex& b, const Vertex& c);
        void AddQuad(const Vertex& topLeft, const Vertex& topRight, const Vertex& bottomLeft, const Vertex& bottomRight);

        // TODO: Factory mesh generation functions

        // Commits all mesh data to GPU resources in preperation for rendering
        void Commit();

        // Rendering methods
        void Draw(const Shader& shader, const Camera& camera, const glm::mat4& transform);
        void DrawInstance(const InstanceData& instanceData);
        void FlushInstances(const Shader& shader, const Camera& camera);

        // Clear all mesh data, cleanup resources, return to initial state
        void Reset();
    
    // Data / implementation
    private:

        // Vertex / index data
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // OpenGL Resources
        VertexAttributes* vertexAttributes = nullptr;
        GPUBuffer* vertexBuffer = nullptr;
        GPUBuffer* indexBuffer = nullptr;
        GPUBuffer* instanceBuffer = nullptr;
};
