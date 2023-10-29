#include "gpubuffer.hpp"

// Constructor
GPUBuffer::GPUBuffer(GLuint size, BufferType type, const void* const data) : size(size)
{
    // Initialize internal buffer
    this->data = new unsigned char[size];
    this->type = type;

    // Initialize buffer object
    glGenBuffers(1, &bufferID);
    
    // Create the data store with glBufferData so it can be resized / orphaned
    switch (type)
    {
        case BufferType::DynamicVertex:
            uploadTarget = GL_ARRAY_BUFFER;
            glBindBuffer(GL_ARRAY_BUFFER, bufferID);
            glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            break;
        
        case BufferType::StaticVertex:
            uploadTarget = GL_ARRAY_BUFFER;
            glBindBuffer(GL_ARRAY_BUFFER, bufferID);
            glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            break;

        case BufferType::Uniform:
            uploadTarget = GL_UNIFORM_BUFFER;
            glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
            glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            break;
    }
}

GPUBuffer::~GPUBuffer()
{
    // Free internal buffer
    delete[] data;

    // Delete OpenGL buffer object
    glDeleteBuffers(1, &bufferID);
}

// Writes a single mat4 into the internal buffer
void GPUBuffer::Write(const glm::mat4& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::mat4))) return;

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + byteOffset);

    // And a pointer to the matrix data
    const GLfloat* const pSource = (const GLfloat* const)glm::value_ptr(value);
    
    // Write each value of the matrix
    for (int i = 0; i < 16; i++)
    {
        *(pData + i) = *(pSource + i);
    }

    byteOffset += sizeof(glm::mat4);
}

// Writes a single vec4 into the internal buffer
void GPUBuffer::Write(const glm::vec4& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::vec4))) return;

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + byteOffset);

    // Write all components
    *(pData) = value.x;
    *(pData + 1) = value.y;
    *(pData + 2) = value.z;
    *(pData + 3) = value.w;

    byteOffset += sizeof(glm::vec4);
}

// Writes data into the internal buffer
void GPUBuffer::Write(const void* const data, GLuint size)
{
    // Ensure buffer has space for write
    if (!CanWrite(size)) return;

    // Copy the data into the buffer
    memcpy((void*)(this->data + byteOffset), data, size);

    // Increase byte offset accordingly
    byteOffset += size;
}

// Flushes the internal buffer to the OpenGL buffer object
void GPUBuffer::Flush()
{
    // Don't bother if buffer hasn't been written to since last flush
    if (byteOffset == 0) return;

    // Upload the data
    glBindBuffer(uploadTarget, bufferID);
    glBufferSubData(uploadTarget, 0, byteOffset, data);
    glBindBuffer(uploadTarget, 0);

    // Reset byte offset
    byteOffset = 0;
}

// Binds the buffer to the specified target
void GPUBuffer::Bind(GLenum target)
{
    glBindBuffer(target, bufferID);
}

// Binds the buffer to the specified target and index
void GPUBuffer::BindBase(GLenum target, GLuint index)
{
    glBindBufferBase(target, index, bufferID);
}