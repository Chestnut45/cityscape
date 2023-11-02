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
void FrameBuffer::AttachTexture(const Texture2D* const texture, GLenum attachment)
{
    // Retrieve ID and bind texture
    GLuint texID = texture->GetID();

    // Attach it to the given attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texID, 0);
}

// Bind this framebuffer to the specified target (or GL_FRAMEBUFFER, if none is supplied)
void FrameBuffer::Bind(GLenum target)
{
    glBindFramebuffer(target, fbo);
}

// Checks this fbo for completeness
// NOTE: This object must be bound
bool FrameBuffer::CheckCompleteness()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: Framebuffer incomplete: 0x" << status << std::endl;
        return false;
    }

    return true;
}