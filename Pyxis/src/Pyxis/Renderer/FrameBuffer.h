#pragma once
#include "Texture.h"

namespace Pyxis
{

	enum class FrameBufferTextureFormat
	{
		None = 0,

		//color
		RGBA8,
		R32I,
		R32UI,

		//depth / stencil
		DEPTH24STENCIL8,

		//defaults
		Depth = DEPTH24STENCIL8,
	};

	enum class FrameBufferTextureType
	{
		None = 0,
		Color,
		Depth,
	};

	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format, FrameBufferTextureType type)
			: TextureFormat(format), TextureType(type)
		{

		}

		FrameBufferTextureFormat TextureFormat;
		FrameBufferTextureType TextureType;

		//could have things like wrap, clamp/ ect
	}; 

	

	struct FrameBufferAttachmentSpecification
	{
		FrameBufferAttachmentSpecification() = default;

		FrameBufferAttachmentSpecification(std::initializer_list<FrameBufferTextureSpecification> attachments)
			: Attachments(attachments) {};


		std::vector<FrameBufferTextureSpecification> Attachments;
	};

	struct FrameBufferSpecification
	{
		uint32_t Width, Height;
		FrameBufferAttachmentSpecification Attachments;
		//idk
		uint32_t samples = 1;
		bool SwapChainTarget = false;
	};


	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() {}

		virtual void Invalidate() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void ReadPixel(uint32_t attachmentIndex, int x, int y, void* data) = 0;
		virtual void ReadPixels(uint32_t attachmentIndex, int x, int y, int width, int height, void* data) = 0;

		virtual void ClearColorAttachment(int index, const void* value) = 0;

		virtual uint32_t GetColorAttachmentRendererID(int index = 0) const = 0;
		//virtual void BindTexture(Ref<Texture2D> texture) const = 0;
		//virtual void UnbindTexture() const = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
	};
}