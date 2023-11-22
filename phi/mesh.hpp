#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "gpubuffer.hpp"
#include "texture2d.hpp"
#include "vertex.hpp"
#include "vertexattributes.hpp"
#include "shader.hpp"

namespace Phi
{
    // Texture unit mapping
    // When rendering a mesh, its textures will be mapped to these units
    // You can use explicit layouts in your shader's samplers to access the textures (4.2+),
    // or set them directly with glUniform1i(samplerLoc, texUnit).
    // Ex: layout(binding = 0) uniform sampler2D myAlbedo1Sampler;
    enum class TexUnit : int
    {
        ALBEDO_1,       // 0
        ALBEDO_2,       // 1
        SPECULAR_1,     // 2
        SPECULAR_2,     // 3
        NORMAL_1,       // 4
        NORMAL_2,       // 5

        MAX_TEXTURES    // 6

        // Unused units: GL_TEXTURE6+
    };

    // SSBO binding points
    enum class SSBOBinding : int
    {
        MaterialBuffer, // 0
        InstanceBuffer, // 1
    };

    // Represents a renderable mesh of arbitrary format
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

            // Vertex data generation
            void AddSurface(const std::vector<Vertex>& vertices, const std::vector<GLuint>* const indices = nullptr);
            void AddTriangle(const Vertex& a, const Vertex& b, const Vertex& c);
            void AddQuad(const Vertex& topLeft, const Vertex& topRight,
                         const Vertex& bottomLeft, const Vertex& bottomRight);

            // Texture loading
            void AddTexture(const std::string& path, TexUnit type);

            // Commits all mesh data to GPU resources in preperation for rendering
            void Commit();

            // Immediately render to the current FBO
            void Draw(const Shader& shader);

            // Immediately render iData.size() instances to the current FBO
            // Binds an SSBO to the 
            template <typename InstanceData>
            void DrawInstances(const Shader& shader, const std::vector<InstanceData>& iData);

            // Clear all mesh data, cleanup resources, return to initial state
            void Reset();
        
        // Data / implementation
        private:

            // Mesh texture data format
            struct Texture
            {
                Texture2D texture;
                TexUnit unit;
            };

            // Mesh data
            std::vector<Vertex> vertices;
            std::vector<GLuint> indices;

            // OpenGL Resources
            Texture* textures[(int)TexUnit::MAX_TEXTURES] = {nullptr};
            VertexAttributes* vertexAttributes = nullptr;
            GPUBuffer* vertexBuffer = nullptr;
            GPUBuffer* indexBuffer = nullptr;

            // Static resources

            // Texture storage for all meshes
            static std::unordered_map<std::string, Texture> loadedTextures;
    };
}