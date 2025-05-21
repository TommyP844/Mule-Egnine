#pragma once

#include "Asset/Asset.h"
#include "Graphics/API/Texture2D.h"

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
		UIFont(Ref<Texture2D> atlas, const std::unordered_map<uint32_t, UIFontGlyph>& glyphs, AssetHandle atlasHandle);
		~UIFont();

		const UIFontGlyph& GetGlyph(uint32_t codepoint) const;
		AssetHandle GetAtlasHandle() const { return mAtlasHandle; }

	private:
		Ref<Texture2D> mFontAtlas;
		std::unordered_map<uint32_t, UIFontGlyph> mGlyphs;
		AssetHandle mAtlasHandle;
	};
}
