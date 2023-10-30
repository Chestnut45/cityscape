#include "vertexattributes.hpp"

// Constructor
VertexAttributes::VertexAttributes()
{
    glGenVertexArrays(1, &vao);
}

// Automagical constructor
// Builds the VAO for you assuming you pack the given vertex format into the provided vertex buffer
// Also associates an index buffer with the vao if one is provided
VertexAttributes::VertexAttributes(VertexFormat format, const GPUBuffer* const vbo, const GPUBuffer* const ebo)
{
    // Ensure we have been handed a vertex buffer
    // NOTE: Throwing an exception here might be a good idea, but I'm wary of any non-RAII resources
    // that might fail to release because of it.
    if (vbo->GetType() != BufferType::DynamicVertex && vbo->GetType() != BufferType::StaticVertex)
    {
        std::cout << "Error: Incorrect buffer type to construct VertexAttributes object" << std::endl;
    }
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->GetName());

    // Construct VAO based on internal format, assume tightly packed
    switch (format)
    {
        case VertexFormat::POS:
            Add(3, GL_FLOAT);
            break;
        
        case VertexFormat::POS_COLOR:
            Add(3, GL_FLOAT);
            Add(4, GL_FLOAT);
            break;
        
        case VertexFormat::POS_COLOR_NORM:
            Add(3, GL_FLOAT);
            Add(4, GL_FLOAT);
            Add(3, GL_FLOAT);
            break;
        
        case VertexFormat::POS_COLOR_NORM_UV:
            Add(3, GL_FLOAT);
            Add(4, GL_FLOAT);
            Add(3, GL_FLOAT);
            Add(2, GL_FLOAT);
            break;
        
        case VertexFormat::POS_COLOR_UV:
            Add(3, GL_FLOAT);
            Add(4, GL_FLOAT);
            Add(2, GL_FLOAT);
            break;
        
        case VertexFormat::POS_NORM:
            Add(3, GL_FLOAT);
            Add(3, GL_FLOAT);
            break;
        
        case VertexFormat::POS_NORM_UV:
            Add(3, GL_FLOAT);
            Add(3, GL_FLOAT);
            Add(2, GL_FLOAT);
            break;
        
        case VertexFormat::POS_UV:
            Add(3, GL_FLOAT);
            Add(2, GL_FLOAT);
            break;
    }

    // Bind index buffer if one was supplied
    if (ebo)
    {
        // This call stores the index buffer object id in the currently bound VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->GetName());
        useIndices = true;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // This is fine since the VAO is already unbound
}

// Destructor
VertexAttributes::~VertexAttributes()
{
    glDeleteVertexArrays(1, &vao);
}

// Adds an attribute and associates the currently bound buffer with that attribute
// NOTE: This object must be bound before any calls to Add(), else they are invalid (undefined behaviour)
void VertexAttributes::Add(GLuint numComponents, GLenum type, GLuint stride, size_t offset)
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