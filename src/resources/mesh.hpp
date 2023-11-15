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

// Represents a renderable mesh
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

        // Vertex data generation / manipulation

        // Adds a surface to the mesh
        template <typename VtxFmt>
        void AddSurface(const std::vector<VtxFmt>& vertices, const std::vector<GLuint>* const indices = nullptr);

        // Commit all mesh data to GPU resources
        void Commit();

        // Clear all mesh data, cleanup resources, return to initial state
        void Reset();

        // Immediate render method
        void Draw(const Shader& shader);

        // Instanced rendering methods
        // NOTE: Assumes IData is a GPUBuffer-writable type
        template <typename IData>
        void DrawInstance(const IData& data);
        void FlushInstances(const Shader& shader);
    
    // Data / implementation
    private:

        // Resources
        VertexAttributes* vao = nullptr;
        GPUBuffer* vertexBuffer = nullptr;
        GPUBuffer* indexBuffer = nullptr;
        GPUBuffer* instanceBuffer = nullptr;

        // Represents a single surface; a set of vertex data
        template <typename VtxFmt>
        struct Surface
        {
            std::vector<VtxFmt> vertices;
            std::vector<GLuint> indices;
        };
};
