#include "mesh.hpp"

namespace Phi
{
    template <typename Vertex>
    Mesh<Vertex>::Mesh()
    {
        
    }

    template <typename Vertex>
    Mesh<Vertex>::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>* const indices)
    {
        this->vertices = vertices;
        useIndices = indices; // nullptr == 0 == false for our bool if not supplied

        // Copy indices if they exist
        if (useIndices)
        {
            this->indices = indices;
        }
    }

    template <typename Vertex>
    Mesh<Vertex>::~Mesh()
    {

    }
}