#pragma once

#include "Core/Core.h"

#include <string>
#include <assert.h>

namespace Mule
{
	enum class UIAnchorAxis
	{
        Top                 = 1 << 1,
        Bottom              = 1 << 2,
        Right               = 1 << 3,
        Left                = 1 << 4,
        CenterHorizontal    = 1 << 5,
        CenterVertical      = 1 << 6,
	};

	MULE_ENUM_OPERATORS(UIAnchorAxis)

    template<>
    std::string ToString(UIAnchorAxis axis)
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

    template<>
    UIAnchorAxis FromString(const std::string& axis)
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