#pragma once

#include "Pyxis/Renderer/RendererAPI.h"

namespace Pyxis
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& VertexArray) override;

	};
}