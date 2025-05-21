#pragma once

#include "Ref.h"
#include "Asset/Asset.h"
#include "UIStyleKey.h"

#include <glm/glm.hpp>

#include <variant>
#include <array>

namespace Mule
{
	class UITheme : public Asset<AssetType::UITheme>
	{
	public:
		UITheme(const std::string& name);
		~UITheme();

		static Ref<UITheme> GetDefault();

		template<typename T>
		void SetValue(UIStyleKey key, const T& value);

		template<typename T>
		T GetValue(UIStyleKey key) const;

	private:
		using UIStyleValue = std::variant<
			bool,
			int,
			float,
			glm::vec2, glm::vec3, glm::vec4,
			glm::ivec2, glm::ivec3, glm::ivec4,
			AssetHandle
		>;

		std::array<UIStyleValue, static_cast<size_t>(UIStyleKey::STYLE_KEY_MAX)> mValues;
	};
}

#include "UITheme.inl"
