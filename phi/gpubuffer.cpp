#include "gpubuffer.hpp"

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

bool GPUBuffer::Write(int value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(int)))
    {
        std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
        return false;
    }

    // Grab a float pointer at the current offset
    GLint* pData = (GLint*)(data + currentByteOffset);
    *(pData) = value;
    currentByteOffset += sizeof(int);

    return true;
}

bool GPUBuffer::Write(float value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(float)))
    {
        std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
        return false;
    }

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + currentByteOffset);
    *(pData) = value;
    currentByteOffset += sizeof(float);

    return true;
}

bool GPUBuffer::Write(const glm::vec2& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::vec2))) 
    {
        std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
        return false;
    }

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + currentByteOffset);

    // Write all components
    *(pData) = value.x;
    *(pData + 1) = value.y;

    currentByteOffset += sizeof(glm::vec2);

    return true;
}

bool GPUBuffer::Write(const glm::vec3& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::vec3))) 
    {
        std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
        return false;
    }

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + currentByteOffset);

    // Write all components
    *(pData) = value.x;
    *(pData + 1) = value.y;
    *(pData + 2) = value.z;

    currentByteOffset += sizeof(glm::vec3);

    return true;
}

bool GPUBuffer::Write(const glm::vec4& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::vec4))) 
    {
        std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
        return false;
    }

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + currentByteOffset);

    // Write all components
    *(pData) = value.x;
    *(pData + 1) = value.y;
    *(pData + 2) = value.z;
    *(pData + 3) = value.w;

    currentByteOffset += sizeof(glm::vec4);

    return true;
}

bool GPUBuffer::Write(const glm::mat4& value)
{
    // Ensure buffer has space for write
    if (!CanWrite(sizeof(glm::mat4))) 
    {
        std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
        return false;
    }

    // Grab a float pointer at the current offset
    GLfloat* pData = (GLfloat*)(data + currentByteOffset);

    // And a pointer to the matrix data
    const GLfloat* const pSource = (const GLfloat* const)glm::value_ptr(value);
    
    // Write each value of the matrix
    for (int i = 0; i < 16; i++)
    {
        *(pData + i) = *(pSource + i);
    }

    currentByteOffset += sizeof(glm::mat4);

    return true;
}

bool GPUBuffer::Write(const void* const data, GLuint size)
{
    // Ensure buffer has space for write
    if (!CanWrite(size)) 
    {
        std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
        return false;
    }

    // Copy the data into the buffer
    memcpy((void*)(this->data + currentByteOffset), data, size);

    // Increase byte offset accordingly
    currentByteOffset += size;

    return true;
}

void GPUBuffer::Flush(bool resetOffset)
{
    glBindBuffer(defaultTarget, bufferID);
    glBufferSubData(defaultTarget, 0, size, data);
    glBindBuffer(defaultTarget, 0);

    // Reset internal buffer offset if requested
    currentByteOffset = resetOffset ? 0 : currentByteOffset;
}

bool GPUBuffer::FlushSection(GLuint offset, GLuint bytes, bool resetOffset)
{
    if (offset + bytes > size)
    {
        std::cout << "ERROR: Buffer section flush failed @" << this << std::endl;
        return false;
    }

    glBindBuffer(defaultTarget, bufferID);
    glBufferSubData(defaultTarget, offset, bytes, data);
    glBindBuffer(defaultTarget, 0);
    
    // Reset internal buffer offset if requested
    currentByteOffset = resetOffset ? 0 : currentByteOffset;

    return true;
}

void GPUBuffer::Bind() const
{
    glBindBuffer(defaultTarget, bufferID);
}

void GPUBuffer::Bind(GLenum target) const
{
    glBindBuffer(target, bufferID);
}

void GPUBuffer::BindBase(GLenum target, GLuint index) const
{
    glBindBufferBase(target, index, bufferID);
}