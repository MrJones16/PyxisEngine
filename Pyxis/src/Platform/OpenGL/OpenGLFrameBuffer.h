#pragma once

#include "Pyxis/Renderer/FrameBuffer.h"
namespace Pyxis
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer();
		virtual ~OpenGLFrameBuffer() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void BindTexture() const override;
		virtual void UnbindTexture() const override;

		virtual Ref<Texture> GetTexture() const override;
	private:
		uint32_t m_RendererID;
	};
}