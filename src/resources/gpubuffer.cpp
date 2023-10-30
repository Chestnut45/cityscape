#include "gpubuffer.hpp"

// Constructor
GPUBuffer::GPUBuffer(BufferType type, GLuint size, const void* const data) : size(size)
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

            defaultTarget = GL_ARRAY_BUFFER;

            glBindBuffer(defaultTarget, bufferID);
            glBufferData(defaultTarget, size, data, GL_DYNAMIC_DRAW);
            glBindBuffer(defaultTarget, 0);
            break;
        
        case BufferType::StaticVertex:

            defaultTarget = GL_ARRAY_BUFFER;

            glBindBuffer(defaultTarget, bufferID);
            glBufferData(defaultTarget, size, data, GL_STATIC_DRAW);
            glBindBuffer(defaultTarget, 0);
            break;
        
        case BufferType::DynamicIndex:

            defaultTarget = GL_ELEMENT_ARRAY_BUFFER;

            glBindBuffer(defaultTarget, bufferID);
            glBufferData(defaultTarget, size, data, GL_DYNAMIC_DRAW);
            glBindBuffer(defaultTarget, 0);
            break;
        
        case BufferType::StaticIndex:

            defaultTarget = GL_ELEMENT_ARRAY_BUFFER;

            glBindBuffer(defaultTarget, bufferID);
            glBufferData(defaultTarget, size, data, GL_STATIC_DRAW);
            glBindBuffer(defaultTarget, 0);
            break;

        case BufferType::Uniform:

            defaultTarget = GL_UNIFORM_BUFFER;

            glBindBuffer(defaultTarget, bufferID);
            glBufferData(defaultTarget, size, data, GL_DYNAMIC_DRAW);
            glBindBuffer(defaultTarget, 0);
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

// Writes a single vec3 into the internal buffer
bool GPUBuffer::Write(const glm::vec3& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::vec3))) return false;

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + byteOffset);

    // Write all components
    *(pData) = value.x;
    *(pData + 1) = value.y;
    *(pData + 2) = value.z;

    byteOffset += sizeof(glm::vec3);

    return true;
}

// Writes a single vec4 into the internal buffer
bool GPUBuffer::Write(const glm::vec4& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::vec4))) return false;

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + byteOffset);

    // Write all components
    *(pData) = value.x;
    *(pData + 1) = value.y;
    *(pData + 2) = value.z;
    *(pData + 3) = value.w;

    byteOffset += sizeof(glm::vec4);

    return true;
}

// Writes a single mat4 into the internal buffer
bool GPUBuffer::Write(const glm::mat4& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::mat4))) return false;

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

    return true;
}

// Writes data into the internal buffer
bool GPUBuffer::Write(const void* const data, GLuint size)
{
    // Ensure buffer has space for write
    if (!CanWrite(size)) return false;

    // Copy the data into the buffer
    memcpy((void*)(this->data + byteOffset), data, size);

    // Increase byte offset accordingly
    byteOffset += size;

    return true;
}

// Flushes the internal buffer to the OpenGL buffer object
void GPUBuffer::Flush()
{
    // Don't bother if buffer hasn't been written to since last flush
    if (byteOffset == 0) return;

    // Upload the data
    glBindBuffer(defaultTarget, bufferID);
    glBufferSubData(defaultTarget, 0, byteOffset, data);
    glBindBuffer(defaultTarget, 0);

    // Reset byte offset
    byteOffset = 0;
}

// Binds the buffer to the default target
void GPUBuffer::Bind()
{
    glBindBuffer(defaultTarget, bufferID);
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