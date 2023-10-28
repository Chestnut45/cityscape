#include "gpubuffer.hpp"

// Constructor
GPUBuffer::GPUBuffer(GLuint size, BufferType type, const void* const data) : size(size)
{
    // Initialize internal buffer
    this->data = new unsigned char[size];

    // Initialize buffer object
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    
    // Create the data store with glBufferData so it can be resized / orphaned
    switch (type)
    {
        case BufferType::Dynamic:
            glBufferData(GL_ARRAY_BUFFER, size, data ? data : NULL, GL_DYNAMIC_DRAW);
            break;
        
        case BufferType::Static:
            glBufferData(GL_ARRAY_BUFFER, size, data ? data : NULL, GL_STATIC_DRAW);
            break;
    }

    // Unbind before returning
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GPUBuffer::~GPUBuffer()
{
    // Free internal buffer
    delete[] data;

    // Delete OpenGL buffer object
    glDeleteBuffers(1, &bufferID);
}

// Writes data into the internal buffer, if it would fit
void GPUBuffer::Write(const void* const data, GLuint size)
{
    // Ensure buffer has space for write
    if (byteOffset + size > this->size)
    {
        std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
        return;
    }

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
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, byteOffset, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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