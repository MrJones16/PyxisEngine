#include "pxpch.h"
#include "Buffer.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
//can use #ifdef PX_PLATFORM_WINDOWS around includes or cases

namespace Pyxis
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:     PX_CORE_ASSERT(false, "RenererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:   return new OpenGLVertexBuffer(vertices, size);
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:     PX_CORE_ASSERT(false, "RenererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:   return new OpenGLIndexBuffer(indices, count);
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}