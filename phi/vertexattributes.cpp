#include "vertexattributes.hpp"

namespace Phi
{
    // Default constructor
    VertexAttributes::VertexAttributes()
    {
        glGenVertexArrays(1, &vao);
    }

    // Automagical constructor
    // Builds the VAO for you assuming you pack the given vertex format into the provided vertex buffer
    // Also associates an index buffer with the vao if one is provided
    VertexAttributes::VertexAttributes(VertexFormat format, const GPUBuffer* const vbo, const GPUBuffer* const ebo)
    {   
        // Bind VAO and VBO
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo->GetName());

        // Construct VAO based on internal format, assume tightly packed
        switch (format)
        {
            case VertexFormat::POS:
                stride = sizeof(VertexPos);
                Add(3, GL_FLOAT);
                break;
            
            case VertexFormat::POS_COLOR:
                stride = sizeof(VertexPosColor);
                Add(3, GL_FLOAT);
                Add(4, GL_FLOAT);
                break;
            
            case VertexFormat::POS_COLOR_NORM:
                stride = sizeof(VertexPosColorNorm);
                Add(3, GL_FLOAT);
                Add(4, GL_FLOAT);
                Add(3, GL_FLOAT);
                break;
            
            case VertexFormat::POS_COLOR_NORM_UV:
                stride = sizeof(VertexPosColorNormUv);
                Add(3, GL_FLOAT);
                Add(4, GL_FLOAT);
                Add(3, GL_FLOAT);
                Add(2, GL_FLOAT);
                break;
            
            case VertexFormat::POS_COLOR_UV:
                stride = sizeof(VertexPosColorUv);
                Add(3, GL_FLOAT);
                Add(4, GL_FLOAT);
                Add(2, GL_FLOAT);
                break;
            
            case VertexFormat::POS_NORM:
                stride = sizeof(VertexPosNorm);
                Add(3, GL_FLOAT);
                Add(3, GL_FLOAT);
                break;
            
            case VertexFormat::POS_NORM_UV:
                stride = sizeof(VertexPosNormUv);
                Add(3, GL_FLOAT);
                Add(3, GL_FLOAT);
                Add(2, GL_FLOAT);
                break;
            
            case VertexFormat::POS_UV:
                stride = sizeof(VertexPosUv);
                Add(3, GL_FLOAT);
                Add(2, GL_FLOAT);
                break;
        }

        // Bind index buffer if one was supplied
        if (ebo)
        {
            // This call stores the index buffer object id in the currently bound VAO
            ebo->Bind(GL_ELEMENT_ARRAY_BUFFER);
            useIndices = true;
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // Destructor
    VertexAttributes::~VertexAttributes()
    {
        glDeleteVertexArrays(1, &vao);
    }

    // Adds an attribute and associates the currently bound buffer with that attribute
    // type must be one of GL_FLOAT, FL_INT, or GL_UNSIGNED_BYTE
    // This object must be bound properly before making any calls to Add()
    void VertexAttributes::Add(GLuint numComponents, GLenum type)
    {
        glVertexAttribPointer(attribCount, numComponents, type, GL_FALSE, stride, (void*)currentOffset);
        glEnableVertexAttribArray(attribCount);

        // Increase counters
        attribCount++;

        switch (type)
        {
            case GL_FLOAT:
                currentOffset += numComponents * sizeof(GLfloat);
                break;
            
            case GL_INT:
                currentOffset += numComponents * sizeof(GLint);
                break;
            
            case GL_UNSIGNED_BYTE:
                currentOffset += numComponents * sizeof(GLubyte);
                break;
        }
    }

    // Binds the VAO
    void VertexAttributes::Bind() const
    {
        glBindVertexArray(vao);
    }
}