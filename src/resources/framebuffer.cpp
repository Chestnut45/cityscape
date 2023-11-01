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

// Bind this framebuffer to the deafult target
void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}