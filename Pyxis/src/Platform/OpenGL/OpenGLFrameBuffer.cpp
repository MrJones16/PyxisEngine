#include "pxpch.h"
#include "OpenGLFrameBuffer.h"

#include <glad/glad.h>

namespace Pyxis
{

	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), count, outID);
		}
		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void CreateRenderBuffers(uint32_t* outID, uint32_t count)
		{
			glCreateRenderbuffers(count, outID);
		}

		static void BindRenderBuffer(GLenum target, uint32_t id)
		{
			glBindRenderbuffer(target, id);
		}


		static void AttachColorTexture(uint32_t id, int samples, GLenum internalformat, GLenum format, GLenum type, uint32_t width, uint32_t height, int index)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalformat, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, nullptr);
				//glTextureStorage2D(id, 1, format, width, height);



				// set filtering things and like wrap
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			}
			//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, id, 0); might work? but not for multisample maybe? 
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
			
		}

		static void AttachDepthRenderBuffer(uint32_t id, GLenum target, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{

			//glTextureStorage2D(id, 1, format, width, height);
			glRenderbufferStorage(target, format, width, height);

			// set filtering things and like wrap
			glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			//glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, target, id);
		}
	}
	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec)
		: m_Specification(spec)
	{
		//handle the types of attachments on creation, and not in invalidation
		for (auto format : spec.Attachments.Attachments)
		{
			//if the texture is a color attachment
			if (format.TextureType == FrameBufferTextureType::Color)
			{
				m_ColorAttachmentSpecifications.emplace_back(format);
			}
			else if (format.TextureType == FrameBufferTextureType::Depth)
			{
				m_DepthAttachmentSpecification = format;
			}
			else
			{
				PX_CORE_ERROR("Frame Buffer was given an attachment with no type! expected color or depth");
			}
		}
		Invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteRenderbuffers(1, &m_DepthAttachment);
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		if (m_RendererID) //already exists, remove previous
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteRenderbuffers(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		//create the framebuffer
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); // TRY THIS

		bool multiSample = m_Specification.samples > 1;

		//Add attachments
		if (m_ColorAttachmentSpecifications.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			Utils::CreateTextures(multiSample, m_ColorAttachments.data(), m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::BindTexture(multiSample, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecifications[i].TextureFormat)
				{
				case FrameBufferTextureFormat::RGBA8:
				{
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.samples, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, m_Specification.Width, m_Specification.Height, i);
					break;
				}
				case FrameBufferTextureFormat::R32I:
				{
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.samples, GL_R32I, GL_RED_INTEGER, GL_INT, m_Specification.Width, m_Specification.Height, i);
					break;
				}
				case FrameBufferTextureFormat::R32UI:
				{
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.samples, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, m_Specification.Width, m_Specification.Height, i);
					break;
				}
				default:
					break;
				}
			}
		}
		
		if (m_DepthAttachmentSpecification.TextureFormat != FrameBufferTextureFormat::None)
		{
			Utils::CreateRenderBuffers(&m_DepthAttachment, 1);
			Utils::BindRenderBuffer(GL_RENDERBUFFER, m_DepthAttachment);
			switch (m_DepthAttachmentSpecification.TextureFormat)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:
			{
				Utils::AttachDepthRenderBuffer(m_DepthAttachment, GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
				break;
			}
			default:
				break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			PX_CORE_ASSERT(m_ColorAttachments.size() <= 4, "Too many color attachments!");

			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachments.size(), buffers);

		}
		else if (m_ColorAttachments.empty())
		{
			//only depth-pass
			glDrawBuffer(GL_NONE);
		}

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

		uint32_t value = -1;
		glClearTexImage(m_ColorAttachments[1], 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &value);
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

	void OpenGLFrameBuffer::ReadPixel(uint32_t attachmentIndex, int x, int y, void* data)
	{
		ReadPixels(attachmentIndex, x, y, 1, 1, data);
	}

	void OpenGLFrameBuffer::ReadPixels(uint32_t attachmentIndex, int x, int y, int width, int height, void* data)
	{
		PX_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Attachment index too great!");

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);

		
		switch (m_ColorAttachmentSpecifications[attachmentIndex].TextureFormat)
		{
		case FrameBufferTextureFormat::None:
		{
			break;
		}
		case FrameBufferTextureFormat::RGBA8:
		{
			glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
			break;
		}
		case FrameBufferTextureFormat::R32I:
		{
			glReadPixels(x, y, width, height, GL_RED_INTEGER, GL_INT, data);
			break;
		}
		case FrameBufferTextureFormat::R32UI:
		{
			glReadPixels(x, y, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
			break;
		}
		default:
		{
			break;
		}
		}
	}

	void OpenGLFrameBuffer::ClearColorAttachment(int index, const void* value)
	{
		PX_CORE_ASSERT(index < m_ColorAttachments.size(), "Attachment index too great!");

		switch (m_ColorAttachmentSpecifications[index].TextureFormat)
		{
		case FrameBufferTextureFormat::RGBA8:
		{
			glClearTexImage(m_ColorAttachments[index], 0, GL_RGBA, GL_UNSIGNED_BYTE, value);
			break;
		}
		case FrameBufferTextureFormat::R32I:
		{
			glClearTexImage(m_ColorAttachments[index], 0, GL_RED_INTEGER, GL_INT, value);
			break;
		}
		case FrameBufferTextureFormat::R32UI:
		{
			glClearTexImage(m_ColorAttachments[index], 0, GL_RED_INTEGER, GL_UNSIGNED_INT, value);
			break;
		}
		case FrameBufferTextureFormat::None:
		default:
		{
			return;
		}
		}
		
	}

}