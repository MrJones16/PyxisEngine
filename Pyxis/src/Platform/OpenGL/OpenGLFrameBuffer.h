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


		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void ReadPixel(uint32_t attachmentIndex, int x, int y, void* data) override;
		virtual void ReadPixels(uint32_t attachmentIndex, int x, int y, int width, int height, void* data) override;

		virtual void ClearColorAttachment(int index, const void* value) override;

		inline virtual uint32_t GetColorAttachmentRendererID(int index) const override { PX_CORE_ASSERT(index < m_ColorAttachments.size(), "Index exceeds the count of attachments!");  return m_ColorAttachments[index]; }
		virtual void BindColorAttachmentTexture(int index = 0) const override;

		virtual const FrameBufferSpecification& GetSpecification() const { return m_Specification; }

	private:
		FrameBufferSpecification m_Specification;

		uint32_t m_RendererID = (uint32_t)0;

		std::vector<FrameBufferTextureSpecification> m_ColorAttachmentSpecifications;
		std::vector<uint32_t> m_ColorAttachments;

		FrameBufferTextureSpecification m_DepthAttachmentSpecification = {FrameBufferTextureFormat::None, FrameBufferTextureType::None};
		uint32_t m_DepthAttachment = 0;
	};
}