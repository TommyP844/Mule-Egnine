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

		UIRect CalculateRect(const UIRect& parentRect, std::optional<UIRect> defaultRect) const;

		void SetLeft(const UIMeasurement& left);
		void SetRight(const UIMeasurement& right);
		void SetTop(const UIMeasurement& top);
		void SetBottom(const UIMeasurement& bottom);
		void SetWidth(const UIMeasurement& width);
		void SetHeight(const UIMeasurement& height);	

	private:
		std::optional<UIMeasurement> mLeft;
		std::optional<UIMeasurement> mRight;
		std::optional<UIMeasurement> mTop;
		std::optional<UIMeasurement> mBottom;

		std::optional<UIMeasurement> mWidth;
		std::optional<UIMeasurement> mHeight;
	};
}