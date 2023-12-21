#include "pxpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Pyxis
{
	Shader* Shader::Create(const std::string& vertexPath, const std::string& fragmentPath)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:     PX_CORE_ASSERT(false, "RenererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:   return new OpenGLShader(vertexPath, fragmentPath);
		}

		PX_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}