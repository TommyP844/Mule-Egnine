#pragma once

#include "UIMeasurement.h"

#include <glm/glm.hpp>

namespace Mule
{
	struct UIRect
	{
		UIRect() = default;
		UIRect(float x, float y, float width, float height)
			:
			x(x), y(y), width(width), height(height)
		{ }
	
		float x;
		float y;
		float width;
		float height;

		glm::vec2 GetPosition() const { return glm::vec2(x, y); }
		glm::vec2 GetSize() const { return glm::vec2(width, height); }
	};
}
