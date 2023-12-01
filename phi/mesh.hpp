#pragma once

#include <iostream>
#include <type_traits> // Important for std::is_same_v
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

        // Unused units: GL_TEXTURE6+ for your custom textures / shaders
    };

    // SSBO binding points
    enum class SSBOBinding : int
    {
        MaterialBuffer, // 0 (Unused for now)
        InstanceBuffer, // 1
    };

    // Represents a renderable mesh of arbitrary format
    template <typename Vertex>
    class Mesh
    {
        // Interface
        public:

            // Manual / procedural constructors
            Mesh();
            Mesh(const std::vector<Vertex>* const vertices, const std::vector<GLuint>* const indices = nullptr);

            // Destructor
            ~Mesh();

            // Delete copy constructor/assignment
            Mesh(const Mesh&) = delete;
            Mesh& operator=(const Mesh&) = delete;

            // Explicit move constructor
            Mesh(Mesh&& other)
            {
                // Steal resources from other mesh
                vertices = other.vertices;
                indices = other.indices;
                useIndices = other.useIndices;
                vertexAttributes = other.vertexAttributes;
                vertexBuffer = other.vertexBuffer;
                indexBuffer = other.indexBuffer;

                // Ensure other mesh doesn't free resources we're stealing on destruction
                other.vertexAttributes = nullptr;
                other.vertexBuffer = nullptr;
                other.indexBuffer = nullptr;

                // Steal all textures and set others to nullptr
                // so the texture refCounts are unaffected
                for (int i = 0; i < (int)TexUnit::MAX_TEXTURES; ++i)
                {
                    textures[i] = other.textures[i];
                    other.textures[i] = nullptr;
                }

                std::cout << "Mesh moved from " << &other << " to " << this << std::endl;

                // Increase refCount so previous object can use regular destructor without breaking static resources
                refCount++;
            };

            // Delete move assignment operator
            void operator=(Mesh&& other) = delete;

            // Vertex data generation
            void AddSurface(const std::vector<Vertex>* vertices, const std::vector<GLuint>* const indices = nullptr);
            void AddTriangle(const Vertex& a, const Vertex& b, const Vertex& c);
            void AddQuad(const Vertex& topLeft, const Vertex& topRight,
                         const Vertex& bottomLeft, const Vertex& bottomRight);

            // Texture loading
            void AddTexture(const std::string& path, TexUnit type);

            // Commits all mesh data to GPU resources in preperation for rendering
            void Commit();

            // Immediately render to the current FBO
            void Draw(const Shader& shader) const;

            // Immediately render iData.size() instances to the current FBO, uploads iData 
            // directly to the static instance buffer and binds it to SSBOBinding::InstanceBuffer
            template <typename InstanceData>
            void DrawInstances(const Shader& shader, const std::vector<InstanceData>& iData) const;

            // Clear all mesh data, cleanup resources, return to initial state
            void Reset();
        
        // Data / implementation
        private:

            friend class Model;

            // Mesh texture data format
            struct Texture
            {
                Texture2D texture;
                TexUnit unit;
                std::string path;
                int refCount = 0;
            };

            // Mesh data
            std::vector<Vertex> vertices;
            std::vector<GLuint> indices;
            bool useIndices;

            // OpenGL Resources
            Texture* textures[(int)TexUnit::MAX_TEXTURES] = {nullptr};
            VertexAttributes* vertexAttributes = nullptr;
            GPUBuffer* vertexBuffer = nullptr;
            GPUBuffer* indexBuffer = nullptr;

            // Static resources

            // Texture storage for all meshes
            static inline std::unordered_map<std::string, Texture> loadedTextures;

            // Instance buffer used by all meshes
            static inline GPUBuffer* instanceBuffer = nullptr;
            static const size_t INSTANCE_BUFFER_SIZE = sizeof(glm::mat4) * 100'000;

            // Reference counter for all meshes
            static inline int refCount = 0;
            static void IncreaseReferences();
            static void DecreaseReferences();
    };

    // Template implementation

    template <typename Vertex>
    Mesh<Vertex>::Mesh()
    {
        std::cout << "Mesh created @" << this <<  std::endl;
        IncreaseReferences();
    }

    template <typename Vertex>
    Mesh<Vertex>::Mesh(const std::vector<Vertex>* vertices, const std::vector<GLuint>* const indices)
    {
        this->vertices = *vertices;
        useIndices = indices; // nullptr == 0 == false for our bool if not supplied

        // Copy indices if they exist
        if (useIndices)
        {
            this->indices = *indices;
        }

        std::cout << "Mesh created @" << this << std::endl;
        IncreaseReferences();
    }

    template <typename Vertex>
    Mesh<Vertex>::~Mesh()
    {
        Reset();
        std::cout << "Mesh destroyed @" << this << std::endl;
        DecreaseReferences();
    }

    template <typename Vertex>
    void Mesh<Vertex>::AddSurface(const std::vector<Vertex>* vertices, const std::vector<GLuint>* const indices)
    {

    }

    template <typename Vertex>
    void Mesh<Vertex>::AddTriangle(const Vertex& a, const Vertex& b, const Vertex& c)
    {

    }

    template <typename Vertex>
    void Mesh<Vertex>::AddQuad(const Vertex& topLeft, const Vertex& topRight,
                               const Vertex& bottomLeft, const Vertex& bottomRight)
    {

    }

    template <typename Vertex>
    void Mesh<Vertex>::AddTexture(const std::string& path, TexUnit type)
    {
        std::cout << "Mesh texture loaded: " << path << std::endl;
    }


    template <typename Vertex>
    void Mesh<Vertex>::Commit()
    {
        // Create VBO and EBO
        vertexBuffer = new GPUBuffer(BufferType::Static, sizeof(Vertex) * vertices.size(), vertices.data());
        indexBuffer = new GPUBuffer(BufferType::Static, sizeof(GLuint) * indices.size(), indices.data());
        
        // VAO creation (Depends on vertex format)
        if (std::is_same_v<Vertex, VertexPosColorNormUv1Uv2>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_NORM_UV1_UV2, vertexBuffer, indexBuffer);
        }

        std::cout << "Mesh resources committed to VRAM" << std::endl;
    }


    template <typename Vertex>
    void Mesh<Vertex>::Draw(const Shader& shader) const
    {
        shader.Use();

        // Bind the VAO
        vertexAttributes->Bind();

        // Bind all textures
        for (Texture* tex : textures)
        {
            if (tex)
            {
                tex->texture.Bind((int)tex->unit);
            }
        }

        // Issue draw call
        if (useIndices)
        {
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }

        // Unbind VAO
        glBindVertexArray(0);
    }


    template <typename Vertex>
    template <typename InstanceData>
    void Mesh<Vertex>::DrawInstances(const Shader& shader, const std::vector<InstanceData>& iData) const
    {
        
    }

    template <typename Vertex>
    void Mesh<Vertex>::Reset()
    {
        vertices.clear();
        indices.clear();

        // Manage static texture resources from our pointer
        for (Texture* tex : textures)
        {
            if (tex)
            {
                tex->refCount--;

                // Delete static resources if we were the last mesh using them
                if (tex->refCount == 0)
                {
                    // This automatically calls the Texture2D object's destructor
                    loadedTextures.erase(tex->path);
                    tex = nullptr;
                    
                    std::cout << "Texture unloaded: unused by any meshes" << std::endl;
                }
            }
        }

        if (vertexAttributes) { delete vertexAttributes; vertexAttributes = nullptr; }
        if (vertexBuffer) { delete vertexBuffer; vertexBuffer = nullptr; }
        if (indexBuffer) { delete indexBuffer; indexBuffer = nullptr; }
    }

    // Reference counting helpers

    template <typename Vertex>
    void Mesh<Vertex>::IncreaseReferences()
    {
        // Initialize static resources for all meshes
        if (refCount == 0)
        {
            std::cout << "First mesh, instance buffer initialized" << std::endl;
            instanceBuffer = new GPUBuffer(BufferType::DynamicDoubleBuffer, INSTANCE_BUFFER_SIZE);
        }

        refCount++;
    }

    template <typename Vertex>
    void Mesh<Vertex>::DecreaseReferences()
    {
        refCount--;

        // Delete static resources if we are the last mesh
        if (refCount == 0)
        {
            std::cout << "Last mesh destroyed, instance buffer deleted" << std::endl;
            delete instanceBuffer;
        }
    }
}