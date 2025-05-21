#pragma once

#include <type_traits> // uint32_t
#include <string>
#include <assert.h>

namespace Mule
{
	enum class UIUnitType : uint32_t
	{
		Pixels,
		Percentage,

		MAX_UNIT_TYPE
	};

	constexpr std::string GetUIUnitTypeString(UIUnitType type)
	{
		switch (type)
		{
		case Mule::UIUnitType::Pixels:		return "Pixels";
		case Mule::UIUnitType::Percentage:	return "Percentage";
		case Mule::UIUnitType::MAX_UNIT_TYPE:
		default:
			assert("Invalid unit type");
			break;
		}
	}

	constexpr UIUnitType GetUIUnitTypeFromString(const std::string& unit)
	{
		if (unit == "Pixels") return UIUnitType::Pixels;
		if (unit == "Percentage") return UIUnitType::Percentage;
		return UIUnitType::MAX_UNIT_TYPE;
	}
}