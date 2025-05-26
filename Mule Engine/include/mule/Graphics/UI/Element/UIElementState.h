#pragma once

#include "Core/Core.h"

#include <array>
#include <assert.h>

namespace Mule
{
	enum class UIElementState : uint32_t
	{
		Idle			= 1 << 1,
		Hovered			= 1 << 2,
		Pressed			= 1 << 3,
		Focused			= 1 << 4,
		Disabled		= 1 << 5,
		Selected		= 1 << 6
	};

	constexpr std::array<UIElementState, 7> AllUIStates = {
		UIElementState::Idle,
		UIElementState::Hovered,
		UIElementState::Pressed,
		UIElementState::Focused,
		UIElementState::Disabled,
		UIElementState::Selected
	};

	MULE_ENUM_OPERATORS(UIElementState);

	template<>
	std::string ToString(UIElementState type)
	{
		switch (type)
		{
		case Mule::UIElementState::Idle:		return "Idle";
		case Mule::UIElementState::Hovered:		return "Hovered";
		case Mule::UIElementState::Pressed:		return "Pressed";
		case Mule::UIElementState::Focused:		return "Focused";
		case Mule::UIElementState::Disabled:	return "Disabled";
		case Mule::UIElementState::Selected:	return "Selected";
		default:
			assert("Invalid UIElementState");
			break;
		}
	}

	template<>
	UIElementState FromString(const std::string& enumValue)
	{
		if (enumValue == "Idle") return UIElementState::Idle;
		if (enumValue == "Hovered") return UIElementState::Hovered;
		if (enumValue == "Pressed") return UIElementState::Pressed;
		if (enumValue == "Focused") return UIElementState::Focused;
		if (enumValue == "Disabled") return UIElementState::Disabled;
		if (enumValue == "Selected") return UIElementState::Selected;

		return UIElementState();
	}
}