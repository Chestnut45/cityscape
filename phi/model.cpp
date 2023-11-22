#include "model.hpp"

namespace Phi
{    
    Model::Model(const std::string& objPath)
    {
        // Create the importer and read the model file
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(objPath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

        // Ensure the scene was imported correctly
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            std::cout << "Error: Assimp: " << importer.GetErrorString() << std::endl;
            return;
        }

        // Process all nodes, starting at the root node
        ProcessNode(scene->mRootNode, scene);
    }

    Model::~Model()
    {

    }

    void Model::Draw(const Shader& shader)
    {

    }

    template <typename InstanceData>
    void Model::DrawInstances(const Shader& shader, const std::vector<InstanceData>& iData)
    {

    }

    void Model::ProcessNode(aiNode* node, const aiScene* scene)
    {
        // Add all meshes from the node
        for(size_t i = 0; i < node->mNumMeshes; ++i)
        {
            AddMesh(scene->mMeshes[node->mMeshes[i]], scene);
        }

        // Process all sub-nodes
        for(size_t i = 0; i < node->mNumChildren; ++i)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    void Model::AddMesh(aiMesh* mesh, const aiScene* scene)
    {
        // Initialize a vector for the vertices
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // Copy all vertices from the mesh
        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex v{0.0f};

            v.x = mesh->mVertices[i].x;
            v.y = mesh->mVertices[i].y;
            v.z = mesh->mVertices[i].z;

            v.nx = mesh->mNormals[i].x;
            v.ny = mesh->mNormals[i].y;
            v.nz = mesh->mNormals[i].z;

            // Add colors if present
            if (mesh->mColors[0])
            {
                v.r = mesh->mColors[0][i].r;
                v.g = mesh->mColors[0][i].g;
                v.b = mesh->mColors[0][i].b;
                v.a = mesh->mColors[0][i].a;
            }

            // Add UVs if present
            if (mesh->mTextureCoords[0])
            {
                v.u1 = mesh->mTextureCoords[0][i].x;
                v.v1 = mesh->mTextureCoords[0][i].y;
            }
            if (mesh->mTextureCoords[1])
            {
                v.u2 = mesh->mTextureCoords[1][i].x;
                v.v2 = mesh->mTextureCoords[1][i].y;
            }
            
            vertices.push_back(v);
        }

        // Copy indices
        for(size_t i = 0; i < mesh->mNumFaces; ++i)
        {
            aiFace face = mesh->mFaces[i];
            for(size_t j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Load material textures
        if (mesh->mMaterialIndex >= 0)
        {

        }
    }
}