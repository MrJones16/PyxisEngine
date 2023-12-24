#pragma once
#include "Texture.h"

namespace Pyxis
{
	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual Ref<Texture2D> GetFrameBufferTexture() const = 0;
		virtual void RescaleFrameBuffer(uint32_t width, uint32_t height) const = 0;
		//virtual void BindTexture(Ref<Texture2D> texture) const = 0;
		//virtual void UnbindTexture() const = 0;

		static Ref<FrameBuffer> Create(uint32_t width, uint32_t height);
	};
}