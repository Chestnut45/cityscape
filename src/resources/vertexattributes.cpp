#include "vertexattributes.hpp"

// Constructor
VertexAttributes::VertexAttributes()
{
    glGenVertexArrays(1, &vao);
}

// Destructor
VertexAttributes::~VertexAttributes()
{
    glDeleteVertexArrays(1, &vao);
}

// Adds an attribute and associates the currently bound buffer with that attribute
// NOTE: This object must be bound before any calls to Add(), else they are invalid (undefined behaviour)
void VertexAttributes::Add(GLuint numComponents, GLenum type, GLuint stride = 0, GLuint offset = 0)
{
    glVertexAttribPointer(attribCount, numComponents, type, GL_FALSE, stride, (void*)offset);
    glEnableVertexAttribArray(attribCount);

    // Increase counters
    attribCount++;
}

// Binds the VAO
void VertexAttributes::Bind()
{
    glBindVertexArray(vao);
}

// Unbinds the VAO
void VertexAttributes::Unbind()
{
    glBindVertexArray(0);
}