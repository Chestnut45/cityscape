#pragma once

#include <vector>

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions

class FrameBuffer
{
    // Interface
    public:
        FrameBuffer();
        ~FrameBuffer();

        // Delete copy constructor/assignment
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        // Delete move constructor/assignment
        FrameBuffer(FrameBuffer&& other) = delete;
        void operator=(FrameBuffer&& other) = delete;

        // Binding state
        void Bind();

    // Data / implementation
    private:
        GLuint fbo;
        std::vector<GLuint> attachments;
};