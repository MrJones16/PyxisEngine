#pragma once

#include <string>
#include "Pyxis/Core/Core.h"

namespace Pyxis
{

	//TODO: Enable custom setting of filtering & texture properties

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual uint8_t* GetData() = 0;
		
		virtual uint32_t GetID() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator== (const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> CreateGlyph(uint32_t width, uint32_t height, unsigned char* buffer);
	private:

	};

	class Texture3D : public Texture
	{
	public:
		static Ref<Texture3D> Create(const std::string& path);
		static Ref<Texture3D> Create(uint32_t width, uint32_t height, uint32_t length);
		virtual uint32_t GetLength() const = 0;
	private:

	};

}