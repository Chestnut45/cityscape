//-----------------------------------------------------------------------------
// File:			RenderTarget.h
// Original Author:	Gordon Wood
//
// Simple little class to wrap FBO functionality for post-processing sample(s).
// This should *not* be considered a good implementation for a wrapper for FBOs,
// and this is why it's not included in wolf. It's used only to make the rest
// of the code here easier to follow
//-----------------------------------------------------------------------------
#ifndef W_RENDERTARGET_H_
#define W_RENDERTARGET_H_

#include "W_Types.h"
#include "W_Material.h"
#include "W_VertexBuffer.h"
#include "W_VertexDeclaration.h"

namespace wolf
{
class RenderTarget
{
	public:
		//-------------------------------------------------------------------------
		// PUBLIC INTERFACE
		//-------------------------------------------------------------------------
		RenderTarget(unsigned int width, unsigned int height, wolf::Texture::Format fmt, bool withDepth = true);
		virtual ~RenderTarget();	

		void SetViewport(unsigned int width, unsigned int height) { m_viewportWidth = width; m_viewportHeight = height; }
		void Bind() const;

		wolf::Texture* GetColorBuffer() const { return m_colorBuffer; }
		wolf::Texture* GetDepthBuffer() const { return m_depthBuffer; }

        float GetViewportWidth() const { return m_viewportWidth; }
        float GetViewportHeight() const { return m_viewportHeight; }

        float GetTextureWidth() const { return m_texWidth; }
        float GetTextureHeight() const { return m_texHeight; }
    
		void SetFilterMode(Texture::FilterMode min, Texture::FilterMode mag)
		{
			if(m_colorBuffer)
				m_colorBuffer->SetFilterMode(min,mag);
		}

		void Blit(wolf::Material* pPostFXMat, RenderTarget* pSrcImage, unsigned int width, unsigned int height) const;
		void Accum(wolf::Material* pPostFXMat, RenderTarget* pSrcImage, unsigned int width, unsigned int height) const;

		static void InitScreen(unsigned int width, unsigned int height);

		static void SetScreenSize(unsigned int width, unsigned int height)
		{
			if(!s_screenFB)
			{
				printf("Error: Screen RenderTarget not initialized\n");
				return;
			}

			s_screenFB->m_viewportWidth = width;
			s_screenFB->m_viewportHeight = height;
		}

		static const RenderTarget& GetScreen() { return *s_screenFB; }
		//-------------------------------------------------------------------------

	private:
		RenderTarget(unsigned int width, unsigned int height, bool isScreen, wolf::Texture::Format fmt, bool withDepth);

		//-------------------------------------------------------------------------
		// PRIVATE MEMBERS
		//-------------------------------------------------------------------------
		GLuint 						m_fbo;
		wolf::Texture*				m_colorBuffer;
		wolf::Texture*				m_depthBuffer;

		float						m_viewportWidth;
		float						m_viewportHeight;
		float						m_texWidth;
		float						m_texHeight;
		glm::vec3					m_pos;
		bool						m_isScreen;
		wolf::Texture::Format		m_fmt;

		static RenderTarget* 			s_screenFB;
		static wolf::VertexBuffer*		s_pVB;
		static wolf::VertexDeclaration*	s_pDecl;
		//-------------------------------------------------------------------------
};
}

#endif

