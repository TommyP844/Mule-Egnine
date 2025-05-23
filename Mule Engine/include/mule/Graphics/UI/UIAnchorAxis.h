#pragma once

#include <string>

namespace Mule
{
	enum class UIAnchorAxis
	{
        Top,
        Bottom,
        Right,
        Left,
        CenterHorizontal,
        CenterVertical,
	};

    constexpr std::string GetUIAnchorAxisString(UIAnchorAxis axis)
    {
        switch (axis)
        {
        case Mule::UIAnchorAxis::Top:                   return "Top";
        case Mule::UIAnchorAxis::Bottom:                return "Bottom";
        case Mule::UIAnchorAxis::Right:                 return "Right";
        case Mule::UIAnchorAxis::Left:                  return "Left";
        case Mule::UIAnchorAxis::CenterHorizontal:      return "CenterHorizontal";
        case Mule::UIAnchorAxis::CenterVertical:        return "CenterVertical";
        default:
            assert("Invalid UIAnchorAxis");
            break;
        }
    }

    constexpr UIAnchorAxis GetUIAnchorAxisFromString(const std::string& axis)
    {
        if(axis == "Top") return UIAnchorAxis::Top;
        if (axis == "Bottom") return UIAnchorAxis::Bottom;
        if (axis == "Right") return UIAnchorAxis::Right;
        if (axis == "Left") return UIAnchorAxis::Left;
        if (axis == "CenterHorizontal") return UIAnchorAxis::CenterHorizontal;
        if (axis == "CenterVertical") return UIAnchorAxis::CenterVertical;

        assert("Invalid UIAnchorAxis Name");
    }
}