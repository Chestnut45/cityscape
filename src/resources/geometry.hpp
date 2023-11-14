#include "../resources/vertexattributes.hpp"
#include "../resources/vertex.hpp"

// Geometric constants
static constexpr float TAU = 6.28318530718;

// Wrapped it in a namespace so globals X and Z are constrained to this scope
namespace Icosphere
{
    // Icosphere data
    static const float X = 0.525731112119133606f;
    static const float Z = 0.850650808352039932f;

    static const VertexPos ICOSPHERE_VERTICES[] =
    {
        {-X, 0.0f, Z}, {X, 0.0f, Z}, {-X, 0.0f, -Z}, {X, 0.0f, -Z},
        {0.0f, Z, X}, {0.0f, Z, -X}, {0.0f, -Z, X}, {0.0f, -Z, -X},
        {Z, X, 0.0f}, {-Z, X, 0.0f}, {Z, -X, 0.0f}, {-Z, -X, 0.0f}
    };

    // NOTE: These indices specify the INTERNAL tris of an icosphere
    // This is so we can avoid changing glCullFace() modes in the render method,
    // since we only want to render the internal faces of light volumes
    // to avoid them disappearing when we enter them
    static const GLuint ICOSPHERE_INDICES[] =
    {
        0, 4, 1,
        0, 9, 4,
        9, 5, 4,
        4, 5, 8,
        4, 8, 1,
        8, 10, 1,
        8, 3, 10,
        5, 3, 8,
        5, 2, 3,
        2, 7, 3,
        7, 10, 3,
        7, 6, 10,
        7, 11, 6,
        11, 0, 6,
        0, 1, 6,
        6, 1, 10,
        9, 0, 11,
        9, 11, 2,
        9, 2, 5,
        7, 2, 11
    };
}