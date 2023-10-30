#pragma once

#include <iostream>
#include "string.h" // For memcpy

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions

enum class BufferType
{
    DynamicVertex,
    StaticVertex,
    DynamicIndex,
    StaticIndex,
    Uniform,
};

class GPUBuffer
{
    // Interface
    public:
        GPUBuffer(BufferType type, GLuint size, const void* const data = NULL);
        ~GPUBuffer();

        // Delete copy constructor/assignment
        GPUBuffer(const GPUBuffer&) = delete;
        GPUBuffer& operator=(const GPUBuffer&) = delete;

        // Delete move constructor/assignment
        GPUBuffer(GPUBuffer&& other) = delete;
        void operator=(GPUBuffer&& other) = delete;

        // Write operations
        bool Write(const glm::vec3& value);
        bool Write(const glm::vec4& value);
        bool Write(const glm::mat4& value);
        bool Write(const void* const data, GLuint size);
        void Flush();

        // State management
        void Bind();
        void Bind(GLenum target);
        void BindBase(GLenum target, GLuint index);

        // Accessors
        inline GLuint GetName() const { return bufferID; };
        inline BufferType GetType() const { return type; };

    // Data / implementation
    private:
        // Should know their own buffer type, small cost
        BufferType type;

        // Internal buffer
        const unsigned char * data = nullptr;
        const GLuint size;

        // Current write offset
        unsigned int byteOffset = 0;

        // OpenGL object handles
        GLuint bufferID;
        GLenum defaultTarget;

        // Helper method to ensure buffer writes are safe
        inline bool CanWrite(size_t bytes) const
        {
            if (byteOffset + bytes > size)
            {
                std::cout << "ERROR: Buffer write failed @" << this << ", would have overflowed" << std::endl;
                return false;
            }
            return true;
        };
};