#pragma once

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions

// RAII VAO wrapper
class VertexAttributes
{
    // Interface
    public:
        VertexAttributes();
        ~VertexAttributes();

        // Delete copy constructor/assignment
        VertexAttributes(const VertexAttributes&) = delete;
        VertexAttributes& operator=(const VertexAttributes&) = delete;

        // Delete move constructor/assignment
        VertexAttributes(VertexAttributes&& other) = delete;
        void operator=(VertexAttributes&& other) = delete;

        // Adds an attribute and associates the currently bound buffer with that attribute
        // NOTE: This object must be bound before any calls to Add(), else they are invalid (undefined behaviour)
        void Add(GLuint numComponents, GLenum type, GLuint stride = 0, GLuint offset = 0);

        // Binding methods
        void Bind();
        void Unbind();

    // Data / implementation
    private:
        // OpenGL objects
        GLuint vao;

        // Counters
        GLuint attribCount = 0;
};