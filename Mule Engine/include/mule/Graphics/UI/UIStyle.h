#pragma once

#include "Ref.h"
#include "Asset/Asset.h"

#include "UIStyleKey.h"

#include <array>
#include <variant>

#include <glm/glm.hpp>

namespace Mule
{
	class UIStyle : public Asset<AssetType::UIStyle>
	{
	public:
		UIStyle(const std::string& name);
		~UIStyle() = default;

		void SetParentStyle(Ref<UIStyle> parentStyle);
		Ref<UIStyle> GetParentStyle() const;

		template<typename T>
		void SetValue(UIStyleKey key, const T& value);

		template<typename T>
		T GetValue(UIStyleKey key) const;

		bool HasValue(UIStyleKey key) const;

	private:
		Ref<UIStyle> mParentStyle;

		using UIStyleValue = std::variant<
			bool,
			int,
			float,
			glm::vec2, glm::vec3, glm::vec4,
			glm::ivec2, glm::ivec3, glm::ivec4
		>;

		std::array<UIStyleValue, static_cast<size_t>(UIStyleKey::STYLE_KEY_MAX)> mValues;
		std::array<bool, static_cast<size_t>(UIStyleKey::STYLE_KEY_MAX)> mValid;
	};
}

#include "UIStyle.inl"