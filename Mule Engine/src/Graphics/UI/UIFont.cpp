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

}