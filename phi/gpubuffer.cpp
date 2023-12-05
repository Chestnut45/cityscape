#include "gpubuffer.hpp"

namespace Phi
{
    GPUBuffer::GPUBuffer(BufferType type, GLuint size, const void* const data) : size(size), type(type)
    {
        glGenBuffers(1, &id);

        switch (type)
        {
            case BufferType::Static:                numSections = 1;    break;
            case BufferType::Dynamic:               numSections = 1;    break;
            case BufferType::DynamicDoubleBuffer:   numSections = 2;    break;
            case BufferType::DynamicTripleBuffer:   numSections = 3;    break;
        }

        // Set the flags for buffer storage creation / mapping
        GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

        // Create an immutable data store
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferStorage(GL_ARRAY_BUFFER, size * numSections, data, flags);

        // Map the buffer
        if (type != BufferType::Static)
        {
            pData = (unsigned char*)glMapBufferRange(GL_ARRAY_BUFFER, 0, size * numSections, flags | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!pData) FatalError("OpenGL: Failed to map buffer");
            pCurrent = pData;
        }

        // Unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GPUBuffer::~GPUBuffer()
    {
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

    void GPUBuffer::Bind(GLenum target) const
    {
        glBindBuffer(target, id);
    }

    void GPUBuffer::BindBase(GLenum target, GLuint index) const
    {
        glBindBufferBase(target, index, id);
    }

    void GPUBuffer::BindRange(GLenum target, GLuint index, GLintptr offset, GLsizeiptr size)
    {
        glBindBufferRange(target, index, id, offset, size);
    }

    void GPUBuffer::Lock()
    {
        // If already locked, delete old sync
        if (syncObj[currentSection])
        {
            glDeleteSync(syncObj[currentSection]);
            syncObj[currentSection] = 0;
        }

        // Place fence sync
        syncObj[currentSection] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    void GPUBuffer::Sync()
    {
        // Only sync if a sync object exists
        if (syncObj[currentSection])
        {
            // Wait for the sync object to be signaled
            while (true)
            {
                GLenum response = glClientWaitSync(syncObj[currentSection], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
                if (response == GL_ALREADY_SIGNALED || response == GL_CONDITION_SATISFIED) return;
            }

            // Delete and reset the sync object
            glDeleteSync(syncObj[currentSection]);
            syncObj[currentSection] = 0;
        }
    }

    void GPUBuffer::SwapSections()
    {
        // Update current section
        currentSection++;
        if (currentSection >= numSections) currentSection = 0;
        pCurrent = pData + currentSection * size;
    }
}