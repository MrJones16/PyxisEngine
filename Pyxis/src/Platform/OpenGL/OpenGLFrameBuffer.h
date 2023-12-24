#pragma once

#include "Pyxis/Renderer/FrameBuffer.h"
namespace Pyxis
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(uint32_t width, uint32_t height);
		virtual ~OpenGLFrameBuffer() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual Ref<Texture2D> GetFrameBufferTexture() const override { return m_texture; }
		virtual void RescaleFrameBuffer(uint32_t width, uint32_t height) const override;

	private:
		uint32_t m_RendererID;
		Ref<Texture2D> m_texture;
		uint32_t m_RenderBufferID;
	};
}