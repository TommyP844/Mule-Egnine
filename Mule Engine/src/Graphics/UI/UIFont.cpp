#include "Graphics/UI/UIFont.h"

namespace Mule
{

    UIFont::UIFont(Ref<Texture2D> atlas, const std::unordered_map<uint32_t, UIFontGlyph>& glyphs, float lineHeight, AssetHandle atlasHandle)
        :
        mFontAtlas(atlas),
        mGlyphs(glyphs),
        mAtlasHandle(atlasHandle),
        mLineHeight(lineHeight)
    {
    }

    UIFont::~UIFont()
    {
    }

    const UIFontGlyph& UIFont::GetGlyph(uint32_t codepoint) const
    {
        return mGlyphs.at(codepoint);
    }

    glm::vec2 UIFont::CalculateSize(const std::string& str, WeakRef<UIStyle> style, WeakRef<UITheme> theme, float wrapWidth)
    {
		float fontSize = style->GetValue<float>(UIStyleKey::FontSize, theme);
		glm::vec2 cursor = glm::vec2(0.f, GetLineHeight() * fontSize);

		for (auto c : str)
		{
			if (c == '\n')
			{
				cursor.x = 0.f;
				cursor.y +=GetLineHeight() * fontSize;
				continue;
			}
			const auto& glyph = GetGlyph(c);

			glm::vec2 min = cursor + glyph.PlaneMin * fontSize;
			glm::vec2 max = cursor + glyph.PlaneMax * fontSize;

			if (max.x > wrapWidth)
			{
				cursor.x = 0.f;
				cursor.y += GetLineHeight() * fontSize;

				min = cursor + glyph.PlaneMin * fontSize;
				max = cursor + glyph.PlaneMax * fontSize;
			}

			if (c == ' ')
			{
				cursor.x += glyph.Advance * fontSize;
				continue;
			}

			cursor.x += glyph.Advance * fontSize;
		}

		return cursor;
    }

}