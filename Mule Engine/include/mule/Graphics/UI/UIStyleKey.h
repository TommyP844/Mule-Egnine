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
		FontSize,

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
		case UIStyleKey::FontSize:
		case UIStyleKey::BorderRadius:
			return UIStyleKeyDataType::Float;

		case UIStyleKey::Padding:
			return UIStyleKeyDataType::Vec2;

		case UIStyleKey::BorderColor:
		case UIStyleKey::BackgroundColor:
		case UIStyleKey::ForegroundColor:
			return UIStyleKeyDataType::Color;

		case UIStyleKey::Font:
			return UIStyleKeyDataType::AssetHandle;

		case UIStyleKey::STYLE_KEY_MAX:
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
		case Mule::UIStyleKey::FontSize:			return "FontSize";
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
		else if (name == "FontSize") return Mule::UIStyleKey::FontSize;

		return UIStyleKey::STYLE_KEY_MAX;
	}
}