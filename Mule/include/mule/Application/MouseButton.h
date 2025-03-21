#pragma once

#include <GLFW/glfw3.h>

namespace Mule
{
	enum class MouseButton : uint32_t
	{
		Left = GLFW_MOUSE_BUTTON_LEFT,
		Right = GLFW_MOUSE_BUTTON_RIGHT,
		Middle = GLFW_MOUSE_BUTTON_MIDDLE,
	};
}
