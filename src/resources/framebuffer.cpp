#include "framebuffer.hpp"

// Constructor
FrameBuffer::FrameBuffer()
{
    glGenFramebuffers(1, &fbo);
}

// Destructor
FrameBuffer::~FrameBuffer()
{
    // Cleanup resources
    glDeleteFramebuffers(1, &fbo);
}

// Attaches a texture to the given attachment point
void FrameBuffer::AttachTexture(const Texture2D& texture, GLenum attachment)
{

}

// Bind this framebuffer to the specified target (or GL_FRAMEBUFFER, if none is supplied)
void FrameBuffer::Bind(GLenum target)
{
    glBindFramebuffer(target, fbo);
}