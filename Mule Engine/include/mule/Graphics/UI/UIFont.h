#pragma once

#include "Asset/Asset.h"
#include "Graphics/API/Texture2D.h"
#include "Graphics/UI/UIRect.h"
#include "Graphics/UI/UIStyle.h"
#include "Graphics/UI/UITheme.h"

#include <unordered_map>

#include <glm/glm.hpp>

namespace Mule
{
	struct UIFontGlyph
	{
		uint32_t Codepoint;
		glm::vec2 MinUV;
		glm::vec2 MaxUV;
		float Advance;
		glm::vec2 PlaneMin, PlaneMax;
	};

	class UIFont : public Asset<AssetType::UIFont>
	{
	public:
		UIFont(Ref<Texture2D> atlas, const std::unordered_map<uint32_t, UIFontGlyph>& glyphs, float lineHeight, AssetHandle atlasHandle);
		~UIFont();

		const UIFontGlyph& GetGlyph(uint32_t codepoint) const;
		AssetHandle GetAtlasHandle() const { return mAtlasHandle; }
		float GetLineHeight() const { return mLineHeight; }

		glm::vec2 CalculateSize(const std::string& str, WeakRef<UIStyle> style, WeakRef<UITheme> theme, float wrapWidth);

	private:
		Ref<Texture2D> mFontAtlas;
		std::unordered_map<uint32_t, UIFontGlyph> mGlyphs;
		AssetHandle mAtlasHandle;
		float mLineHeight;
	};
}
