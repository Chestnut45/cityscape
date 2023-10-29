#pragma once

#include <iostream>
#include "string.h" // For memcpy

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions

enum class BufferType
{
    Dynamic,
    Static
};

class GPUBuffer
{
    // Interface
    public:
        // Initialization
        GPUBuffer(GLuint size, BufferType type, const void* const data = NULL);
        ~GPUBuffer();

        // Delete copy constructor/assignment
        GPUBuffer(const GPUBuffer&) = delete;
        GPUBuffer& operator=(const GPUBuffer&) = delete;

        // Delete move constructor/assignment
        GPUBuffer(GPUBuffer&& other) = delete;
        void operator=(GPUBuffer&& other) = delete;

        // Write operations
        void Write(const void* const data, GLuint size);
        void Flush();

        // State management
        void Bind(GLenum target);
        void BindBase(GLenum target, GLuint index);

        // Accessors
        inline GLuint GetName() const { return bufferID; };

    // Data / implementation
    private:
        // Internal buffer
        const unsigned char * data = nullptr;
        unsigned int byteOffset = 0;
        const GLuint size;

        // OpenGL buffer object handle
        GLuint bufferID;
};