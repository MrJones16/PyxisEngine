#include "pxpch.h"
#include "Texture.h"

#include "Pyxis/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Pyxis
{
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:     PX_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:   return CreateRef<OpenGLTexture2D>(path);
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:     PX_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:   return CreateRef<OpenGLTexture2D>(width, height);
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::CreateGlyph(uint32_t width, uint32_t rows, unsigned char* buffer)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:     PX_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:   return CreateRef<OpenGLTexture2D>(width, rows, buffer);
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture3D> Texture3D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:     PX_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:   return CreateRef<OpenGLTexture3D>(path);
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture3D> Texture3D::Create(uint32_t width, uint32_t height, uint32_t length)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:     PX_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLTexture3D>(width, height, length);
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}