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
// NOTE: This object must be bound before any calls to AttachTexture()
void FrameBuffer::AttachTexture(const Texture2D& texture, GLenum attachment)
{
    // Retrieve ID and bind texture
    GLuint texID = texture.GetID();
    glBindTexture(GL_TEXTURE_2D, texID);

    // Attach it to the given attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texID, 0);
}

// Bind this framebuffer to the specified target (or GL_FRAMEBUFFER, if none is supplied)
void FrameBuffer::Bind(GLenum target)
{
    glBindFramebuffer(target, fbo);
}