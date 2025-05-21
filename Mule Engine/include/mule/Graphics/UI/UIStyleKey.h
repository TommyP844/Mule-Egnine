#pragma once

#include <type_traits> // uint64_t
#include <string>
#include <assert.h>

namespace Mule
{
	enum class UIStyleKey : uint64_t
	{
		BorderColor,
		BorderRadius,
		BackgroundColor,
		ForegroundColor,
		Padding,
		Font,

		STYLE_KEY_MAX
	};

	enum class UIStyleKeyDataType : uint64_t
	{
		Bool,
		Integer,
		Float,
		Vec2,
		Vec3,
		Color,
		Ivec2,
		IVec3,
		Ivec4,
		AssetHandle
	};

	constexpr UIStyleKeyDataType GetUIStyleKeyDataType(UIStyleKey key)
	{
		switch (key)
		{
		case Mule::UIStyleKey::BorderRadius:
			return UIStyleKeyDataType::Float;

		case Mule::UIStyleKey::Padding:
			return UIStyleKeyDataType::Vec2;

		case Mule::UIStyleKey::BorderColor:
		case Mule::UIStyleKey::BackgroundColor:
		case Mule::UIStyleKey::ForegroundColor:
			return UIStyleKeyDataType::Color;

		case Mule::UIStyleKey::Font:
			return UIStyleKeyDataType::AssetHandle;

		case Mule::UIStyleKey::STYLE_KEY_MAX:
		default:
			assert(false && "Invalid UIStyleKey");
			break;
		}
	}

	constexpr std::string GetUIStyleKeyName(UIStyleKey key)
	{
		switch (key)
		{
		case Mule::UIStyleKey::BorderColor:		return "Border Color";
		case Mule::UIStyleKey::BorderRadius:	return "Border Radius";
		case Mule::UIStyleKey::BackgroundColor:	return "Background Color";
		case Mule::UIStyleKey::ForegroundColor:	return "Foreground Color";
		case Mule::UIStyleKey::Padding:			return "Padding";
		case Mule::UIStyleKey::Font:			return "Font";
		case Mule::UIStyleKey::STYLE_KEY_MAX:
		default:
			assert(false && "Invalid UIStyleKey");
			break;
		}
	}

	constexpr UIStyleKey GetUIStyleKeyFromName(const std::string& name)
	{
		if (name == "Border Color") return Mule::UIStyleKey::BorderColor;
		else if (name == "Border Radius") return Mule::UIStyleKey::BorderRadius;
		else if (name == "Background Color") return Mule::UIStyleKey::BackgroundColor;
		else if (name == "Foreground Color") return Mule::UIStyleKey::ForegroundColor;
		else if (name == "Padding") return Mule::UIStyleKey::Padding;
		else if (name == "Font") return Mule::UIStyleKey::Font;

		return UIStyleKey::STYLE_KEY_MAX;
	}
}