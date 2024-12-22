#include "Font.h"

namespace Pyxis
{

	//if (FT_Init_FreeType(&ft))
	//{
	//	std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	//	return -1;
	//}
	Font::Font(std::string name, std::string pathToFont)
	{
		//initialize freetype
		FT_Library FTLib;
		if (FT_Init_FreeType(&FTLib))
		{
			PX_CORE_ERROR("FREETYPE: Could not init FreeType Library");
			return;
		}

		//create a freetype face
		FT_Face face;
		if (FT_New_Face(FTLib, pathToFont.c_str(), 0, &face))
		{
			PX_CORE_ERROR("FREETYPE: Failed to load font");
		}

		FT_Set_Pixel_Sizes(face, 0, m_CharacterHeight);

		//we will use the first 128 ascii characters
		for (unsigned char c = 0; c < 128; c++)
		{
			//load the character in the face
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				PX_CORE_ERROR("FREETYPE: Failed To Load Glyph");
				continue;
			}
			//get a 8-bit texture for the glyph
			Ref<Texture2D> texture =  Texture2D::CreateGlyph(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			m_Characters.insert(std::pair<char, Character>(c, character));
		}

		FT_Done_Face(face);
		FT_Done_FreeType(FTLib);

	}

	std::map<std::string, Ref<Font>> FontLibrary::s_FontMap;

	Ref<Font> FontLibrary::AddFont(std::string name, std::string pathToFont)
	{
		if (s_FontMap.contains(name))
		{
			PX_CORE_WARN("Font {0} is already in the library!", name);
			return s_FontMap[name];
		}

		//add this font to the map
		s_FontMap[name] = CreateRef<Font>(name, pathToFont);

		return s_FontMap[name];

	}

	Ref<Font> FontLibrary::GetFont(std::string name)
	{
		if (!s_FontMap.contains(name))
		{
			return nullptr;
		}
		return s_FontMap[name];
	}

}