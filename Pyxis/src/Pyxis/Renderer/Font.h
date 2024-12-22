#pragma once
#include <Pyxis/Core/Core.h>
#include <Pyxis/Renderer/Texture.h>
#include <Pyxis/Core/Log.h>
#include <map>
#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H  

namespace Pyxis
{
	class Font
	{
	public:
		Font(std::string name, std::string pathToFont);

		std::string m_FontName = "font";

		unsigned int m_CharacterHeight = 16;

		struct Character {
			Ref<Texture2D> Texture;  // ID handle of the glyph texture
			glm::ivec2   Size;       // Size of glyph
			glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
			unsigned int Advance;    // Offset to advance to next glyph
		};

		std::map<char, Character> m_Characters;
		Ref<Texture2D> m_BitMap;

	};

	class FontLibrary
	{
		static FT_Library s_FTLibrary;
		static std::map<std::string, Ref<Font>> s_FontMap;

	public:
		
		static Ref<Font> AddFont(std::string name, std::string pathToFont);
		static Ref<Font> GetFont(std::string name);
	};

	
}