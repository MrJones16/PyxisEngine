#include "pxpch.h"
#include "OpenGLFrameBuffer.h"

#include <glad/glad.h>

namespace Pyxis
{
	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec)
		: m_Specification(spec)
	{
		Invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteRenderbuffers(1, &m_ColorAttatchment);
		glDeleteRenderbuffers(1, &m_DepthStencilAttatchment);
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		if (m_RendererID) //already exists, just resizing
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_ColorAttatchment);
			//glDeleteRenderbuffers(1, &m_ColorAttatchment);
			glDeleteRenderbuffers(1, &m_DepthStencilAttatchment);
		}
		//create the framebuffer
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); // TRY THIS

		//create the color render buffer

		/*glCreateRenderbuffers(1, &m_ColorAttatchment);
		glBindRenderbuffer(GL_RENDERBUFFER, m_ColorAttatchment);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_Specification.Width, m_Specification.Height);*/

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttatchment);
		glTextureStorage2D(m_ColorAttatchment, 1, GL_RGBA8, m_Specification.Width, m_Specification.Height);

		glBindTexture(GL_TEXTURE_2D, m_ColorAttatchment);

		glTextureParameteri(m_ColorAttatchment, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_ColorAttatchment, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_ColorAttatchment, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_ColorAttatchment, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttatchment, 0);

		//create the depth / stencil render buffer
		glCreateRenderbuffers(1, &m_DepthStencilAttatchment);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilAttatchment);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height);

		//attach renderbuffer to framebuffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilAttatchment);

		PX_CORE_ASSERT((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "Framebuffer is not complete!");

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			PX_CORE_ERROR("Framebuffer not complete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void OpenGLFrameBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
		Invalidate();
	}

}