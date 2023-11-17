#pragma once

#include <iostream>
#include <string.h> // For memcpy

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h> // OpenGL types / functions

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

        // Accessors
        inline GLuint GetOffset() const { return currentByteOffset; };
        inline GLuint GetSize() const { return size; };

        // Sets the internal buffer's current offset
        inline void SetOffset(GLuint offset) { currentByteOffset = offset < size ? offset : currentByteOffset; };

        // Write operations
        // NOTE: All writes are performed at the internal buffer's current offset
        // If the write succeeds, the current offset will be increased by the size of the data written
        bool Write(int value);
        bool Write(float value);
        bool Write(const glm::vec2& value);
        bool Write(const glm::vec3& value);
        bool Write(const glm::vec4& value);
        bool Write(const glm::mat4& value);
        bool Write(const void* const data, GLuint size);

        // Flush all or part of the internal buffer to the OpenGL buffer object
        void Flush(bool resetOffset = false);
        bool FlushSection(GLuint offset, GLuint bytes, bool resetOffset = false);

        // State management
        void Bind() const;
        void Bind(GLenum target) const;
        void BindBase(GLenum target, GLuint index) const;

        // Accessors
        inline GLuint GetName() const { return bufferID; };
        inline BufferType GetType() const { return type; };

        // Helper method to ensure buffer writes are safe
        inline bool CanWrite(GLuint bytes) const { return (currentByteOffset + bytes <= size); };

    // Data / implementation
    private:
        // Should know their own buffer type, small cost
        BufferType type;

        // Internal buffer
        unsigned char * data = nullptr;
        GLuint size;

        // Current write offset
        GLuint currentByteOffset = 0;

        // OpenGL object handles
        GLuint bufferID;
        GLenum defaultTarget;
};