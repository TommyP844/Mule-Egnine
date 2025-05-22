#pragma once

#include "UIMeasurement.h"
#include "UIRect.h"

#include <glm/glm.hpp>

#include <optional>

namespace Mule
{
	class UITransform
	{
	public:
		UITransform() = default;
		~UITransform() = default;

		std::optional<UIMeasurement> Left;
		std::optional<UIMeasurement> Right;
		std::optional<UIMeasurement> Top;
		std::optional<UIMeasurement> Bottom;

		std::optional<UIMeasurement> Width;
		std::optional<UIMeasurement> Height;
	};
}