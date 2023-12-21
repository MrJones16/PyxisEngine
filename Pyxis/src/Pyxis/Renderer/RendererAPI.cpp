#include "pxpch.h"
#include "RendererAPI.h"

namespace Pyxis
{
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	//VertexArray* VertexArray::Create()
	//{
	//	switch (Renderer::GetAPI())
	//	{
	//	case RendererAPI::None:     PX_CORE_ASSERT(false, "RenererAPI::None is currently not supported!"); return nullptr;
	//	case RendererAPI::OpenGL:   return new OpenGLVertexArray();
	//	}

	//	PX_CORE_ASSERT(false, "Unknown RendererAPI!");
	//	return nullptr;
	//}
}