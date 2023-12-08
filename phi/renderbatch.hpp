#pragma once

#include "mesh.hpp"
#include "gpubuffer.hpp"
#include "texture2d.hpp"
#include "vertex.hpp"
#include "vertexattributes.hpp"
#include "shader.hpp"

namespace Phi
{
    // Dynamic vertex data batching and rendering logic
    // Usage:
    // 1. Have some meshes that share a vertex format
    // 2. Create RenderBatch instance with initial buffer size
    // 3. Every Frame:
    // 4 a. AddMesh() all the meshes you want to draw
    // 4 b. Flush()
    template <typename Vertex>
    class RenderBatch
    {
        // Interface
        public:

            RenderBatch(size_t maxVertices, size_t maxIndices = 0);
            ~RenderBatch();
            
            // Delete copy constructor/assignment
            RenderBatch(const RenderBatch&) = delete;
            RenderBatch& operator=(const RenderBatch&) = delete;

            // Delete move constructor/assignment
            RenderBatch(RenderBatch&& other) = delete;
            void operator=(RenderBatch&& other) = delete;

            // Batching / rendering methods
            void AddMesh(const Mesh<Vertex>& mesh);
            void Flush(const Shader& shader);
        
        // Data / implementation
        private:

            // Local vertex data
            std::vector<Vertex> vertices;
            std::vector<GLuint> indices;

            // State / stats
            size_t maxVertices;
            bool useIndices;

            // OpenGL Resources
            GPUBuffer* vertexBuffer = nullptr;
            GPUBuffer* indexBuffer = nullptr;
            VertexAttributes* vertexAttributes = nullptr;
    };

    // Templated code implementation

    template <typename Vertex>
    RenderBatch<Vertex>::RenderBatch(size_t maxVertices, size_t maxIndices) : maxVertices(maxVertices)
    {
        // Use indices if user supplies an index buffer size
        useIndices = maxIndices != 0;

        // Initialize resources
        vertexBuffer = new GPUBuffer(BufferType::DynamicDoubleBuffer, maxVertices * sizeof(Vertex));

        if (useIndices)
        {
            indexBuffer = new GPUBuffer(BufferType::DynamicDoubleBuffer, maxIndices * sizeof(GLuint));
        }
        
        // VAO creation (Depends on vertex format)
        if (std::is_same_v<Vertex, VertexPos>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        else if (std::is_same_v<Vertex, VertexPosColor>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        else if (std::is_same_v<Vertex, VertexPosColorNorm>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_NORM, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        else if (std::is_same_v<Vertex, VertexPosColorNormUv>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_NORM_UV, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        else if (std::is_same_v<Vertex, VertexPosColorNormUv1Uv2>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_NORM_UV1_UV2, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        else if (std::is_same_v<Vertex, VertexPosColorUv>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_UV, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        else if (std::is_same_v<Vertex, VertexPosNorm>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_NORM, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        else if (std::is_same_v<Vertex, VertexPosNormUv>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_NORM_UV, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        else if (std::is_same_v<Vertex, VertexPosUv>)
        {
            vertexAttributes = new VertexAttributes(VertexFormat::POS_UV, vertexBuffer, useIndices ? indexBuffer : nullptr);
        }
        
        if (!vertexAttributes)
        {
            FatalError("RenderBatch Constructor: Custom vertex format is not supported yet, please use one of the internal vertex formats");
        }
    }

    template <typename Vertex>
    RenderBatch<Vertex>::~RenderBatch()
    {
        // Free all OpenGL resources
        delete vertexBuffer;
        if (indexBuffer) delete indexBuffer;
        delete vertexAttributes;
    }

    template <typename Vertex>
    void RenderBatch<Vertex>::AddMesh(const Mesh<Vertex>& mesh)
    {
        // Get vertex count before new data is appended
        GLuint vertexCount = vertices.size();

        // Copy vertex data
        std::copy(mesh.GetVertices().cbegin(), mesh.GetVertices().cend(), vertices.end());

        if (useIndices)
        {
            // Offset the indices before appending them to the batch
            std::transform(mesh.GetIndices().cbegin(), mesh.GetIndices().cend(), indices.end(), [&vertexCount](GLuint original) { return original + vertexCount; });
        }
    }

    template <typename Vertex>
    void RenderBatch<Vertex>::Flush(const Shader& shader)
    {
        if (useIndices)
        {
            // Ensure OpenGL is not reading from this section of our buffers
            vertexBuffer->Sync();
            indexBuffer->Sync();

            // Write the batch data to the buffers
            vertexBuffer->Write(vertices.data(), vertices.size() * sizeof(Vertex));
            indexBuffer->Write(indices.data(), indices.size() * sizeof(GLuint));

            // Bind resources
            vertexAttributes->Bind();
            shader.Use();

            // Issue draw call
            glDrawElementsBaseVertex(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT,
                                    (void*)((size_t)indexBuffer->GetSize() * indexBuffer->GetCurrentSection()),
                                    maxVertices * vertexBuffer->GetCurrentSection());
            
            // Unbind VAO
            glBindVertexArray(0);
        
            // Insert a fence sync
            vertexBuffer->Lock();
            indexBuffer->Lock();

            // Needed for double-buffering
            vertexBuffer->SwapSections();
            indexBuffer->SwapSections();
        }
        else
        {
            // Ensure OpenGL is not reading from this section of the buffer
            vertexBuffer->Sync();

            // Write the batch data to the buffer
            vertexBuffer->Write(vertices.data(), vertices.size() * sizeof(Vertex));

            // Bind resources
            vertexAttributes->Bind();
            shader.Use();

            // Issue draw call
            // TODO: Test this
            glDrawArrays(GL_TRIANGLES, maxVertices * vertexBuffer->GetCurrentSection(), vertices.size());
            
            // Unbind VAO
            glBindVertexArray(0);
        
            // Insert a fence sync
            vertexBuffer->Lock();

            // Needed for double-buffering
            vertexBuffer->SwapSections();
        }
    }
}