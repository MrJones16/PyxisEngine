#pragma once

#include "Pyxis/Renderer/Texture.h"

#include <glad/glad.h>

namespace Pyxis
{

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(uint32_t width, uint32_t height);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual uint8_t* GetData() override;

		virtual uint32_t GetID() const override { return m_RendererID; }

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override 
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		};
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};

	class OpenGLTexture3D : public Texture3D
	{
	public:
		OpenGLTexture3D(const std::string& path);
		OpenGLTexture3D(uint32_t width, uint32_t height, uint32_t length);
		virtual ~OpenGLTexture3D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetLength() const override { return m_Length; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual uint8_t* GetData() override;
		virtual uint32_t GetID() const override { return m_RendererID; }

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture3D&)other).m_RendererID;
		};
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height, m_Length;
		uint32_t m_RendererID;
	};

}