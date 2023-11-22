#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include "mesh.hpp"

namespace Phi
{
    class Model
    {
        // Interface
        public:
            
            // Internal vertex format for models
            typedef VertexPosColorNormUv1Uv2 Vertex;

            Model(const std::string& objPath);
            ~Model();

            // Delete copy constructor/assignment
            Model(const Model&) = delete;
            Model& operator=(const Model&) = delete;

            // Delete move constructor/assignment
            Model(Model&& other) = delete;
            void operator=(Model&& other) = delete;

            // Immediately render to the current FBO
            void Draw(const Shader& shader);

            // Immediately render iData.size() instances of the model to the current FBO, uploads iData 
            // directly to the static instance buffer and binds it to SSBOBinding::InstanceBuffer
            template <typename InstanceData>
            void DrawInstances(const Shader& shader, const std::vector<InstanceData>& iData);

        // Data / implementation
        private:
            
            std::vector<Mesh<Vertex>> meshes;

            // Static resources
            
            // Instance buffer used by all models
            static inline GPUBuffer* instanceBuffer = nullptr;

            // Helper assimp loading / processing methods
            void ProcessNode(aiNode* node, const aiScene* scene);
            void AddMesh(aiMesh* mesh, const aiScene* scene);
    };
}