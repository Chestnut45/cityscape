#pragma once

#include <vector>

#include "../../thirdparty/glew/include/GL/glew.h" // OpenGL types / functions

#include "texture2d.hpp"

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

        // Attaches a texture to the given attachment point
        void AttachTexture(const Texture2D* const texture, GLenum attachment);

        // Bind the framebuffer (defaults to both read and draw)
        void Bind(GLenum target = GL_FRAMEBUFFER) const;

        // Checks if the framebuffer is complete
        bool CheckCompleteness() const;

    // Data / implementation
    private:
        // OpenGL objects
        GLuint fbo;

        // State
        int width = 0;
        int height = 0;
};