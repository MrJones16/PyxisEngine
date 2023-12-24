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

		virtual void BindTexture() const = 0;
		virtual void UnbindTexture() const = 0;

		virtual Ref<Texture> GetTexture() const = 0;

		static Ref<FrameBuffer> Create();
	};
}