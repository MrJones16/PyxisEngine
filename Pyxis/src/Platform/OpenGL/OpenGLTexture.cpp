#include "pxpch.h"
#include "OpenGLTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>

namespace Pyxis
{
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		stbi_set_flip_vertically_on_load(true);
		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		PX_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else if (channels == 2)
		{
			internalFormat = GL_RG8;
			dataFormat = GL_RG;
		}
		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		PX_CORE_ASSERT(internalFormat && dataFormat, "Format not Supported");

		//create texture in glfw / glad
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		//make room on gpu?
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		//create the texture
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t rows, unsigned char* buffer)
	{


		m_Width = width;
		m_Height = rows;

		m_InternalFormat = GL_RED;
		m_DataFormat = GL_RED;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			m_InternalFormat,
			m_Width,
			m_Height,
			0,
			m_DataFormat,
			GL_UNSIGNED_BYTE,
			buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


		
		//m_Width = width;
		//m_Height = rows;

		//
		//m_InternalFormat = GL_RED;
		//m_DataFormat = GL_RED;

		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		////create texture in glfw / glad
		//glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		////make room on gpu?
		//glTextureStorage2D(m_RendererID, 1, GL_RED, m_Width, m_Height);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, buffer);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t BytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		PX_CORE_ASSERT(size == m_Width * m_Height * BytesPerPixel, "Data must be entire texture");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	uint8_t* OpenGLTexture2D::GetData()
	{
		uint32_t BytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		uint8_t* pixels = new uint8_t[m_Width * m_Height * BytesPerPixel];
		Bind();
		glGetTexImage(GL_TEXTURE_2D, 0, m_DataFormat, GL_UNSIGNED_BYTE, pixels);
		return pixels;
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	OpenGLTexture3D::OpenGLTexture3D(const std::string& path)
		: m_Path(path)
	{
		stbi_set_flip_vertically_on_load(true);
		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		PX_CORE_ASSERT(data, "Failed to load image!");
		PX_CORE_INFO("Channels: {0}", channels);
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else if (channels == 2)
		{
			internalFormat = GL_RG8;
			dataFormat = GL_RG;
		}
		PX_CORE_ASSERT(internalFormat && dataFormat, "Format not Supported");

			//create texture in glfw / glad
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		//make room on gpu?
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	OpenGLTexture3D::OpenGLTexture3D(uint32_t width, uint32_t height, uint32_t length)
	{

	}

	OpenGLTexture3D::~OpenGLTexture3D()
	{

	}

	void OpenGLTexture3D::SetData(void* data, uint32_t size)
	{

	}

	uint8_t* OpenGLTexture3D::GetData()
	{
		return nullptr;
	}

	void OpenGLTexture3D::Bind(uint32_t slot) const
	{

	}
}

