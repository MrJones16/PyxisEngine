#include "Font.h"

namespace Pyxis {

// if (FT_Init_FreeType(&ft))
//{
//	std::cout << "ERROR::FREETYPE: Could not init FreeType Library" <<
//std::endl; 	return -1;
// }
Font::Font(std::string filePath) : Resource(filePath) {
    // initialize freetype
    FT_Library FTLib;
    if (FT_Init_FreeType(&FTLib)) {
        PX_CORE_ERROR("FREETYPE: Could not init FreeType Library");
        return;
    }

    // create a freetype face
    FT_Face face;
    if (FT_New_Face(FTLib, filePath.c_str(), 0, &face)) {
        PX_CORE_ERROR("FREETYPE: Failed to load font");
    }

    FT_Set_Pixel_Sizes(face, 0, m_CharacterHeight);

    // we will use the first 128 ascii characters
    for (unsigned char c = 0; c < 128; c++) {
        // load the character in the face
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            PX_CORE_ERROR("FREETYPE: Failed To Load Glyph");
            continue;
        }
        // get a 8-bit texture for the glyph
        Ref<Texture2D> texture = Texture2D::CreateGlyph(
            face->glyph->bitmap.width, face->glyph->bitmap.rows,
            face->glyph->bitmap.buffer);
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)};
        m_Characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(FTLib);
}

} // namespace Pyxis
