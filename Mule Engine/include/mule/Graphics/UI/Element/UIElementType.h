#pragma once

#include "Core/Core.h"

#include <string>
#include <assert.h>

namespace Mule
{
	enum class UIElementType : uint32_t
	{
		UIText,
		UIButton,

		MAX_UI_ELEMENT_TYPE
	};

	template<>
	std::string ToString(UIElementType type)
	{
		switch (type)
		{
		case Mule::UIElementType::UIText: return "UIText";
		case Mule::UIElementType::UIButton: return "UIButton";
		case Mule::UIElementType::MAX_UI_ELEMENT_TYPE:
		default:
			assert("Invalid UIElementType");
			return "Invalid";
		}
	}

	template<>
	UIElementType FromString(const std::string& type)
	{
		if (type == "UIText") return UIElementType::UIText;
		if (type == "UIButton") return UIElementType::UIButton;
		return UIElementType::MAX_UI_ELEMENT_TYPE;
	}
}