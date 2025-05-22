#pragma once

#include <type_traits> // uint64_t
#include <string>
#include <assert.h>

namespace Mule
{
	enum class UIStyleKey : uint64_t
	{
		BorderRadius,
		BackgroundColor,
		ForegroundColor,
		Padding,
		Font,
		FontSize,
		HasBorder,
		BorderColor,
		BorderWidth,

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
		case UIStyleKey::HasBorder:
			return UIStyleKeyDataType::Bool;

		case UIStyleKey::BorderWidth:
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
		case UIStyleKey::BorderColor:		return "Border Color";
		case UIStyleKey::BorderRadius:		return "Border Radius";
		case UIStyleKey::BackgroundColor:	return "Background Color";
		case UIStyleKey::ForegroundColor:	return "Foreground Color";
		case UIStyleKey::Padding:			return "Padding";
		case UIStyleKey::Font:				return "Font";
		case UIStyleKey::FontSize:			return "Font Size";
		case UIStyleKey::HasBorder:			return "Has Border";
		case UIStyleKey::BorderWidth:		return "Border Width";
		case Mule::UIStyleKey::STYLE_KEY_MAX:
		default:
			assert(false && "Invalid UIStyleKey");
			break;
		}
	}

	constexpr UIStyleKey GetUIStyleKeyFromName(const std::string& name)
	{
		if (name == "Border Color")				return UIStyleKey::BorderColor;
		else if (name == "Border Radius")		return UIStyleKey::BorderRadius;
		else if (name == "Background Color")	return UIStyleKey::BackgroundColor;
		else if (name == "Foreground Color")	return UIStyleKey::ForegroundColor;
		else if (name == "Padding")				return UIStyleKey::Padding;
		else if (name == "Font")				return UIStyleKey::Font;
		else if (name == "Font Size")			return UIStyleKey::FontSize;
		else if (name == "Has Border")			return UIStyleKey::HasBorder;
		else if (name == "Border Width")		return UIStyleKey::BorderWidth;

		return UIStyleKey::STYLE_KEY_MAX;
	}
}