#include "pxpch.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Pyxis
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:     PX_CORE_ASSERT(false, "RenererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::OpenGL:   return new OpenGLVertexArray();
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}