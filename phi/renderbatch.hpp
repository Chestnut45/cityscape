#pragma once

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

            // NOTE: If you are using one of the internal vertex formats,
            // you may omit the vao argument and it will create / manage
            // the resource automatically. This is the preferred method.
            //
            // If you are using a custom format, you must create your own
            // VertexAttributes object, and by passing its address to this
            // constructor, you are *GIVING OWNERSHIP* of that pointer to
            // the RenderBatch instance you are creating.
            //
            // The RenderBatch will take care of freeing / deallocating all
            // resources - including the VertexAttributes object - on destruction.
            RenderBatch(size_t maxVertices, size_t maxIndices, VertexAttributes* vao = nullptr);
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

            // State
            bool useIndices;

            // OpenGL Resources
            GPUBuffer* vertexBuffer = nullptr;
            GPUBuffer* indexBuffer = nullptr;
            VertexAttributes* vertexAttributes = nullptr;
    };

    // Templated code implementation

    template <typename Vertex>
    RenderBatch<Vertex>::RenderBatch(size_t maxVertices, size_t maxIndices, VertexAttributes* vao) : useIndices(useIndices)
    {
        // Initialize resources
        vertexBuffer = new GPUBuffer(BufferType::DynamicDoubleBuffer, maxVertices * sizeof(Vertex));
        indexBuffer = new GPUBuffer(BufferType::DynamicDoubleBuffer, maxIndices * sizeof(GLuint));
        
        // VAO creation (Depends on vertex format)
        if (vao)
        {
            // Ownership transfer
            vertexAttributes = vao;
        }
        else
        {
            // User has an internal vertex format, make a VAO for them
            if (std::is_same_v<Vertex, VertexPos>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS, vertexBuffer, indexBuffer);
            }
            else if (std::is_same_v<Vertex, VertexPosColor>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR, vertexBuffer, indexBuffer);
            }
            else if (std::is_same_v<Vertex, VertexPosColorNorm>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_NORM, vertexBuffer, indexBuffer);
            }
            else if (std::is_same_v<Vertex, VertexPosColorNormUv>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_NORM_UV, vertexBuffer, indexBuffer);
            }
            else if (std::is_same_v<Vertex, VertexPosColorNormUv1Uv2>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_NORM_UV1_UV2, vertexBuffer, indexBuffer);
            }
            else if (std::is_same_v<Vertex, VertexPosColorUv>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS_COLOR_UV, vertexBuffer, indexBuffer);
            }
            else if (std::is_same_v<Vertex, VertexPosNorm>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS_NORM, vertexBuffer, indexBuffer);
            }
            else if (std::is_same_v<Vertex, VertexPosNormUv>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS_NORM_UV, vertexBuffer, indexBuffer);
            }
            else if (std::is_same_v<Vertex, VertexPosUv>)
            {
                vertexAttributes = new VertexAttributes(VertexFormat::POS_UV, vertexBuffer, indexBuffer);
            }
            
            if (!vertexAttributes)
            {
                // Uh-oh, the user has lied to us. Point them in the right direction
                FatalError("RenderBatch constructor: Custom vertex format requires a VertexAttributes* to be passed");
            }
        }
    }

    template <typename Vertex>
    RenderBatch<Vertex>::~RenderBatch()
    {

    }
}