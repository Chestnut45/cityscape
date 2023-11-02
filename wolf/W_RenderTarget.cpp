//-----------------------------------------------------------------------------
// File:			RenderTarget.cpp
// Original Author:	Gordon Wood
//
// See header for notes
//-----------------------------------------------------------------------------
#include "W_RenderTarget.h" 
#include "W_BufferManager.h"

struct QuadVertex
{
	GLfloat x,y;
	GLfloat u,v;
};

static QuadVertex s_quadVertices[] = 
{
	{ -1.0f, -1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f, 0.0f },
	{ -1.0f, 1.0f, 0.0f, 1.0f },

	{ -1.0f, 1.0f, 0.0f, 1.0f },
	{ 1.0f, -1.0f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f }
};


static unsigned int nextPow2(unsigned int p_ui)
{
	p_ui--;
	p_ui |= p_ui >> 1;
	p_ui |= p_ui >> 2;
	p_ui |= p_ui >> 4;
	p_ui |= p_ui >> 8;
	p_ui |= p_ui >> 16;
	p_ui++;
	return p_ui;
}

namespace wolf
{

RenderTarget* 				RenderTarget::s_screenFB;
wolf::VertexBuffer*			RenderTarget::s_pVB;
wolf::VertexDeclaration*	RenderTarget::s_pDecl;

RenderTarget::RenderTarget(unsigned int width, unsigned int height, wolf::Texture::Format fmt, bool withDepth)
	: RenderTarget(width, height, false, fmt, withDepth)
{

}

RenderTarget::RenderTarget(unsigned int width, unsigned int height, bool isScreen, wolf::Texture::Format fmt, bool withDepth)
	: m_fbo(0), m_colorBuffer(nullptr), m_depthBuffer(nullptr), m_viewportWidth(width), m_viewportHeight(height), m_isScreen(isScreen), m_fmt(fmt)
{
	m_texWidth = nextPow2(width);
	m_texHeight = nextPow2(height);

	if( !isScreen )
	{
		glGenFramebuffers(1,&m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		if(fmt != wolf::Texture::FMT_Depth)
		{
			m_colorBuffer = wolf::TextureManager::CreateRenderTexture(m_texWidth, m_texHeight, m_fmt);
			glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, m_colorBuffer->GetGLTexture(), 0);
		}
		else 
		{
			glDrawBuffer(GL_NONE);
		}

		if(fmt == wolf::Texture::FMT_Depth || withDepth)
		{
			m_depthBuffer = wolf::TextureManager::CreateRenderTexture(m_texWidth, m_texHeight, wolf::Texture::FMT_Depth);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer->GetGLTexture(), 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

RenderTarget::~RenderTarget()
{
	if( this != s_screenFB )
	{
		wolf::TextureManager::DestroyTexture(m_colorBuffer);
		if(m_depthBuffer)
			wolf::TextureManager::DestroyTexture(m_depthBuffer);
		glDeleteFramebuffers(1, &m_fbo);
	}
}

void RenderTarget::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport( 0, 0, m_viewportWidth, m_viewportHeight);
}

void RenderTarget::Blit(wolf::Material* pPostFXMat, RenderTarget* pSrcImage, unsigned int width, unsigned int height) const
{
	Bind();
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pPostFXMat->SetTexture("u_srcImage", pSrcImage->GetColorBuffer());
	pPostFXMat->SetUniform("u_scaleOfs", glm::vec4(
		(float)width / (float)pSrcImage->GetTextureWidth(),
		(float)height / (float)pSrcImage->GetTextureHeight(),
		0.0f,
		0.0f));

	pPostFXMat->Apply();
	s_pDecl->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderTarget::Accum(wolf::Material* pPostFXMat, RenderTarget* pSrcImage, unsigned int width, unsigned int height) const
{
	Bind();
	pPostFXMat->SetTexture("u_srcImage", pSrcImage->GetColorBuffer());
	pPostFXMat->SetUniform("u_scaleOfs", glm::vec4(
		(float)width / (float)pSrcImage->GetTextureWidth(),
		(float)height / (float)pSrcImage->GetTextureHeight(),
		0.0f,
		0.0f));

	pPostFXMat->Apply();
	s_pDecl->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderTarget::InitScreen(unsigned int width, unsigned int height)
{
	s_screenFB = new RenderTarget(width, height, true, wolf::Texture::FMT_8888, true);

    s_pVB = wolf::BufferManager::CreateVertexBuffer(s_quadVertices, sizeof(QuadVertex) * 6);

	s_pDecl = new wolf::VertexDeclaration();
	s_pDecl->Begin();
	s_pDecl->AppendAttribute(wolf::AT_Position, 2, wolf::CT_Float);
	s_pDecl->AppendAttribute(wolf::AT_TexCoord1, 2, wolf::CT_Float);
	s_pDecl->SetVertexBuffer(s_pVB);
	s_pDecl->End();
}

}
