#pragma once

#include <string>
#include "Pyxis/Core/Core.h"
#include <Pyxis/Core/Resource.h>

namespace Pyxis
{

	//TODO: Enable custom setting of filtering & texture properties

	class Texture
	{
	public:
		enum FilterMode
		{
			Nearest,
			Linear
		};

		enum WrapMode
		{
			Repeat,
			RepeatMirrored,
			ClampToEdge,
			ClampToBorder
		};

		enum TextureMode
		{
			Stretch,
			Tile
		};

		struct TextureSpecification
		{
			TextureMode m_TextureModeS = Stretch;
			TextureMode m_TextureModeT = Stretch;

			WrapMode m_WrapS = WrapMode::Repeat;
			WrapMode m_WrapT = WrapMode::Repeat;

			FilterMode m_MinFiltering = FilterMode::Nearest;
			FilterMode m_MagFiltering = FilterMode::Nearest;
		};
		
	public:
		

		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual uint8_t* GetData() = 0;
		virtual uint32_t GetID() const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void SetTextureSpecification(TextureSpecification spec) = 0;
		virtual TextureSpecification& GetTextureSpecification() { return m_Specification; };

		virtual bool operator== (const Texture& other) const = 0;
	protected:
		TextureSpecification m_Specification;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path, TextureSpecification spec = TextureSpecification());
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, TextureSpecification spec = TextureSpecification());
		static Ref<Texture2D> CreateGlyph(uint32_t width, uint32_t height, unsigned char* buffer, TextureSpecification spec = TextureSpecification());
	private:
	};

	class Texture3D : public Texture
	{
	public:
		static Ref<Texture3D> Create(const std::string& path, TextureSpecification spec = TextureSpecification());
		static Ref<Texture3D> Create(uint32_t width, uint32_t height, uint32_t length, TextureSpecification spec = TextureSpecification());
		virtual const uint32_t GetLength() const = 0;
	private:

	};

	class Texture2DResource : public Resource
	{
	public:
		Texture2DResource(std::string filePath) : Resource(filePath)
		{
			m_Texture = Texture2D::Create(filePath);
		}
		Ref<Texture2D> m_Texture;
	};

}