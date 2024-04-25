#pragma once

#include "Pyxis/Renderer/FrameBuffer.h"
namespace Pyxis
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer() override;

		virtual void Invalidate() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual uint32_t GetColorAttatchmentRendererID() const override { return m_ColorAttatchment; }
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual const FrameBufferSpecification& GetSpecification() const { return m_Specification; }

	private:
		FrameBufferSpecification m_Specification;

		uint32_t m_RendererID = (uint32_t)0;
		uint32_t m_ColorAttatchment = 0;
		uint32_t m_DepthStencilAttatchment = 0;
	};
}