#include "gpubuffer.hpp"

namespace Phi
{
    GPUBuffer::GPUBuffer(BufferType type, GLuint size, const void* const data) : size(size), type(type)
    {
        glGenBuffers(1, &id);

        GLbitfield mapFlags = 0;

        // OLD
        pData = new unsigned char[size];
        pCurrent = pData;
        
        // Create the data store with glBufferData so it can be resized / orphaned
        switch (type)
        {
            case BufferType::DynamicVertex:

                glBindBuffer(GL_ARRAY_BUFFER, id);
                glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                break;
            
            case BufferType::StaticVertex:

                glBindBuffer(GL_ARRAY_BUFFER, id);
                glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                break;
            
            case BufferType::DynamicIndex:

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                break;
            
            case BufferType::StaticIndex:

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                break;

            case BufferType::Uniform:

                glBindBuffer(GL_UNIFORM_BUFFER, id);
                glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                break;
        }
    }

    GPUBuffer::~GPUBuffer()
    {
        // Free internal buffer
        delete[] pData;

        // Delete OpenGL buffer object
        glDeleteBuffers(1, &id);
    }

    bool GPUBuffer::Write(int value)
    {
        // Ensure buffer has space for write
        if (!CanWrite(sizeof(GLint)))
        {
            std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
            return false;
        }

        *(GLint*)pCurrent = (GLint)value;

        pCurrent += sizeof(GLint);

        return true;
    }

    bool GPUBuffer::Write(float value)
    {
        // Ensure buffer has space for write
        if (!CanWrite(sizeof(GLfloat)))
        {
            std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
            return false;
        }

        *(GLfloat*)pCurrent = (GLfloat)value;

        pCurrent += sizeof(GLfloat);

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

        *(GLfloat*)pCurrent = value.x;
        *(GLfloat*)(pCurrent + sizeof(GLfloat)) = value.y;

        pCurrent += sizeof(glm::vec2);

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

        *(GLfloat*)pCurrent = value.x;
        *(GLfloat*)(pCurrent + sizeof(GLfloat)) = value.y;
        *(GLfloat*)(pCurrent + sizeof(GLfloat) * 2) = value.z;

        pCurrent += sizeof(glm::vec3);

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

        *(GLfloat*)pCurrent = value.x;
        *(GLfloat*)(pCurrent + sizeof(GLfloat)) = value.y;
        *(GLfloat*)(pCurrent + sizeof(GLfloat) * 2) = value.z;
        *(GLfloat*)(pCurrent + sizeof(GLfloat) * 3) = value.w;

        pCurrent += sizeof(glm::vec4);

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

        // And a pointer to the matrix data
        const GLfloat* const pSource = (const GLfloat* const)glm::value_ptr(value);
        
        // Write each value of the matrix
        for (int i = 0; i < 16; i++)
        {
            *(GLfloat*)(pCurrent + i * sizeof(GLfloat)) = *(pSource + i);
        }

        pCurrent += sizeof(glm::mat4);

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
        memcpy(pCurrent, data, size);

        pCurrent += size;

        return true;
    }

    void GPUBuffer::Flush(bool resetOffset)
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, pData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Reset internal buffer pointer if requested
        pCurrent = resetOffset ? pData : pCurrent;
    }

    bool GPUBuffer::FlushSection(GLuint offset, GLuint bytes, bool resetOffset)
    {
        if (offset + bytes > size)
        {
            std::cout << "ERROR: Buffer section flush failed @" << this << std::endl;
            return false;
        }

        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferSubData(GL_ARRAY_BUFFER, offset, bytes, pData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Reset internal buffer pointer if requested
        pCurrent = resetOffset ? pData : pCurrent;

        return true;
    }

    void GPUBuffer::Bind(GLenum target) const
    {
        glBindBuffer(target, id);
    }

    void GPUBuffer::BindBase(GLenum target, GLuint index) const
    {
        glBindBufferBase(target, index, id);
    }
}