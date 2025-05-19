#pragma once

#include "UIMeasurement.h"

namespace Mule
{
	struct UIRect
	{
		UIRect() = default;
		UIRect(float x, float y, float width, float height)
			:
			X(x), Y(y), Width(width), Height(height)
		{ }
	
		float X;
		float Y;
		float Width;
		float Height;
	};
}
