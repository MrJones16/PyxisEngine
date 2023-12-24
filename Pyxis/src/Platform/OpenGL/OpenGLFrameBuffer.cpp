#include "pxpch.h"
#include "OpenGLFrameBuffer.h"

#include <glad/glad.h>

namespace Pyxis
{
	OpenGLFrameBuffer::OpenGLFrameBuffer()
	{
		glCreateFramebuffers(1, &m_RendererID);
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
	}

	void OpenGLFrameBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::BindTexture() const
	{

	}

	void OpenGLFrameBuffer::UnbindTexture() const
	{

	}

	Ref<Texture> OpenGLFrameBuffer::GetTexture() const
	{
		return Ref<Texture>();
	}

}