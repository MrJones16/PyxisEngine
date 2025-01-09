#include "pxpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Pyxis
{
	void OpenGLRendererAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
	}
	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void OpenGLRendererAPI::EnableDepthTest()
	{
		glEnable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::DisableDepthTest()
	{
		glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& VertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : VertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLRendererAPI::DrawArray(const Ref<VertexArray>& VertexArray)
	{
		glDrawArrays(GL_TRIANGLES, 0, 2);
	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& VertexArray, uint32_t VertexCount)
	{
		VertexArray->Bind();
		glDrawArrays(GL_LINES, 0, VertexCount);
	}

	void OpenGLRendererAPI::BindTexture2D(const uint32_t textureID)
	{
		glBindTextureUnit(0, textureID);
		//glBindTexture(GL_TEXTURE_2D, textureID);
	}

}
