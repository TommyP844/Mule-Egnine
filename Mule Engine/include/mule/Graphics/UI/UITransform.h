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

		std::optional<UIMeasurement>& GetLeft() { return mLeft; }
		std::optional<UIMeasurement>& GetTop() { return mTop; }
		std::optional<UIMeasurement>& GetBottom() { return mBottom; }
		std::optional<UIMeasurement>& GetRight() { return mRight; }
		std::optional<UIMeasurement>& GetWidth() { return mWidth; }
		std::optional<UIMeasurement>& GetHeight() { return mHeight; }

		const std::optional<UIMeasurement>& GetLeft() const { return mLeft; }
		const std::optional<UIMeasurement>& GetTop() const { return mTop; }
		const std::optional<UIMeasurement>& GetBottom() const { return mBottom; }
		const std::optional<UIMeasurement>& GetRight() const { return mRight; }
		const std::optional<UIMeasurement>& GetWidth() const { return mWidth; }
		const std::optional<UIMeasurement>& GetHeight() const { return mHeight; }

	private:
		std::optional<UIMeasurement> mLeft;
		std::optional<UIMeasurement> mRight;
		std::optional<UIMeasurement> mTop;
		std::optional<UIMeasurement> mBottom;

		std::optional<UIMeasurement> mWidth;
		std::optional<UIMeasurement> mHeight;
	};
}