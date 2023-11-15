#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "gpubuffer.hpp"
#include "texture2d.hpp"
#include "vertex.hpp"
#include "vertexattributes.hpp"
#include "shader.hpp"

// Represents a renderable mesh of any internal vertex format
template <typename Vertex>
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

        // Procedural geometry generation
        void AddSurface(const std::vector<Vertex>& vertices, const std::vector<GLuint>* const indices = nullptr);
        void AddTriangle(const Vertex& a, const Vertex& b, const Vertex& c);
        void AddQuad();

        // TODO: Factory mesh generation functions

        // Commits all mesh data to GPU resources in preperation for rendering
        void Commit();

        // Rendering methods
        void Draw(const Shader& shader);

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
