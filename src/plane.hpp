#pragma once
#include "../thirdparty/glm/glm/glm.hpp"
#include "../thirdparty/glm/glm/gtc/type_ptr.hpp"
#include "../thirdparty/glew/include/GL/glew.h"
#include <stdio.h>
#include <vector>

// Extremely basic vertex format
struct Vertex
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

// An interface for creating, positioning, scaling, coloring, and rendering planes
class Plane
{
    // Interface
    public:
        Plane(GLuint subdivisions, GLuint shader);
        ~Plane();

        void Draw(GLuint shader);

        // Temporal manipulation
        void SetTranslation(GLfloat x, GLfloat y, GLfloat z);
        void Translate(GLfloat x, GLfloat y, GLfloat z);
        void SetRotation(GLfloat x, GLfloat y, GLfloat z);
        void Rotate(GLfloat x, GLfloat y, GLfloat z);
        void SetScale(GLfloat x, GLfloat y, GLfloat z);
        void Scale(GLfloat x, GLfloat y, GLfloat z);

        // Color manipulation
        void SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

        // Logical state
        glm::mat4 GetTransform() const;
        glm::vec4 GetColor() const;

    // Data / implementation
    private:
        // Plane obj state
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;
        glm::vec4 color;
        GLuint subdivisions;

        // OpenGL objects
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;

        // Vertex data
        GLuint vertexCount = 0;
        GLuint indexCount = 0;

        // Uniform locations
        static inline GLuint colorLoc = 0;
        static inline GLuint modelLoc = 0;
};