#include "pxpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Pyxis
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}