#pragma once
#include "Texture.h"

namespace Pyxis
{
	struct FrameBufferSpecification
	{
		uint32_t Width, Height;

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

		virtual uint32_t GetColorAttatchmentRendererID() const = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		//virtual void BindTexture(Ref<Texture2D> texture) const = 0;
		//virtual void UnbindTexture() const = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
	};
}