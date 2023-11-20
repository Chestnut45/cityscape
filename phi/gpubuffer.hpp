#pragma once

#include <iostream>
#include <string.h> // For memcpy

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h> // OpenGL types / functions

#include "app.hpp" // Error functions

namespace Phi
{
    // Different types of buffers for different usage patterns
    // Dynamic = Persistent + coherent mapping, fast to update with large batches of data
    // Static = Immutable
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

            // Sets the internal buffer's current offset
            inline void SetOffset(GLuint offset) { pCurrent = offset < size ? (pData + currentSection * size + offset) : pCurrent; };

            // Write operations
            // NOTE: All writes are performed at the current pointer offset (in bytes)
            // If the write succeeds, the offset will be increased by the size of the data written
            bool Write(int value);
            bool Write(float value);
            bool Write(const glm::vec2& value);
            bool Write(const glm::vec3& value);
            bool Write(const glm::vec4& value);
            bool Write(const glm::mat4& value);
            bool Write(const void* const data, GLuint size);

            // Binding methods
            void Bind(GLenum target) const;
            void BindBase(GLenum target, GLuint index) const;

            // Synchronization
            void Lock(); // Insert a fence sync for all rendering commands
            void Sync(); // Wait until our sync object has been signaled
            void SwapSections(); // Increase the buffer section, wraps to [0, numSections)

            // Accessors
            inline GLuint GetName() const { return id; };
            inline BufferType GetType() const { return type; };
            inline GLuint GetCurrentSection() const { return currentSection; };
            inline GLuint GetOffset() const { return (pCurrent - (pData + currentSection * size)); };
            inline GLuint GetSize() const { return size; };

            // Helper method to ensure buffer writes are safe
            inline bool CanWrite(GLuint bytes) const { return (pCurrent + bytes) <= (pData + currentSection * size + size); };

        // Data / implementation
        private:
        
            // Should know their own buffer type, small cost
            BufferType type;
            GLuint size;

            // Pointer to persistently mapped buffer
            unsigned char* pData = nullptr;
            unsigned char* pCurrent = nullptr;

            // OpenGL object handles
            GLuint id = 0;
            GLsync syncObj[2] = {0};

            // Section state
            GLuint currentSection = 0;
            GLuint numSections = 1;
    };
}