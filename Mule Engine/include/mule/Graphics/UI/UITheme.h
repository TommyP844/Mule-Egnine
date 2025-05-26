#pragma once

#include "Ref.h"
#include "Asset/Asset.h"
#include "Element/UIElementState.h"

// Styles
#include "Graphics/UI/Style/UIButtonStyle.h"
#include "Graphics/UI/Style/UITextStyle.h"

#include <glm/glm.hpp>

#include <variant>
#include <array>
#include <unordered_map>

namespace Mule
{
	class UITheme : public Asset<AssetType::UITheme>
	{
	public:
		UITheme();
		~UITheme();

		static Ref<UITheme> GetDefault();

		Ref<UIButtonStyle> ButtonStyle;
		Ref<UITextStyle> TextStyle;
	};
}
