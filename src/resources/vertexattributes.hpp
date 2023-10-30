#pragma once

#include <iostream>

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions

#include "gpubuffer.hpp"
#include "vertex.hpp"

// RAII VAO wrapper
class VertexAttributes
{
    // Interface
    public:
        VertexAttributes();
        VertexAttributes(VertexFormat format, const GPUBuffer* const vbo, const GPUBuffer* const ebo = nullptr);
        ~VertexAttributes();

        // Delete copy constructor/assignment
        VertexAttributes(const VertexAttributes&) = delete;
        VertexAttributes& operator=(const VertexAttributes&) = delete;

        // Delete move constructor/assignment
        VertexAttributes(VertexAttributes&& other) = delete;
        void operator=(VertexAttributes&& other) = delete;

        // Sets the stride for all subsequent calls to Add()
        inline void SetStride(GLuint stride) { this->stride = stride; };

        // Adds an attribute and associates the currently bound buffer with that attribute
        // NOTE: This object must be bound before any calls to Add(), else they are invalid (undefined behaviour)
        void Add(GLuint numComponents, GLenum type);

        // Binding methods
        void Bind();
        void Unbind();

    // Data / implementation
    private:
        // OpenGL objects
        GLuint vao;

        // Counters
        GLuint attribCount = 0;
        GLsizeiptr currentOffset = 0;
        GLuint stride = 0;

        // Info
        bool useIndices = false;
};