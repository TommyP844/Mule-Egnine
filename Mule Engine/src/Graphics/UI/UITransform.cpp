#include "Graphics/UI/UITransform.h"

namespace Mule

{

	UIRect UITransform::CalculateRect(const UIRect& parentRect, std::optional<UIRect> defaultRect) const
	{
		float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;

		// Horizontal layout
		std::optional<float> left = mLeft ? std::make_optional(mLeft->Resolve(parentRect.Width)) : std::nullopt;
		std::optional<float> right = mRight ? std::make_optional(mRight->Resolve(parentRect.Width)) : std::nullopt;
		std::optional<float> w = mWidth ? std::make_optional(mWidth->Resolve(parentRect.Width)) : std::nullopt;

		if (left && w)
		{
			x = parentRect.X + *left;
			width = *w;
		}
		else if (left && right)
		{
			x = parentRect.X + *left;
			width = parentRect.Width - *left - *right;
		}
		else if (w && right)
		{
			width = *w;
			x = parentRect.X + parentRect.Width - *right - width;
		}
		else if (left && defaultRect)
		{
			x = parentRect.X + *left;
			width = defaultRect->Width;
		}
		else if (defaultRect)
		{
			x = defaultRect->X;
			width = defaultRect->Width;
		}
		else
		{
			x = parentRect.X;
			width = parentRect.Width;
		}

		// Vertical layout
		std::optional<float> top = mTop ? std::make_optional(mTop->Resolve(parentRect.Height)) : std::nullopt;
		std::optional<float> bottom = mBottom ? std::make_optional(mBottom->Resolve(parentRect.Height)) : std::nullopt;
		std::optional<float> h = mHeight ? std::make_optional(mHeight->Resolve(parentRect.Height)) : std::nullopt;

		if (top && h)
		{
			y = parentRect.Y + *top;
			height = *h;
		}
		else if (top && bottom)
		{
			y = parentRect.Y + *top;
			height = parentRect.Height - *top - *bottom;
		}
		else if (h && bottom)
		{
			height = *h;
			y = parentRect.Y + parentRect.Height - *bottom - height;
		}
		else if (top && defaultRect)
		{
			y = parentRect.Y + *top;
			height = defaultRect->Height;
		}
		else if (defaultRect)
		{
			y = defaultRect->Y;
			height = defaultRect->Height;
		}
		else
		{
			y = parentRect.Y;
			height = parentRect.Height;
		}

		return UIRect{ x, y, width, height };
	}

	void UITransform::SetLeft(const UIMeasurement& left)
	{
		mLeft = left;
	}

	void UITransform::SetRight(const UIMeasurement& right)
	{
		mRight = right;
	}

	void UITransform::SetTop(const UIMeasurement& top)
	{
		mTop = top;
	}

	void UITransform::SetBottom(const UIMeasurement& bottom)
	{
		mBottom = bottom;
	}

	void UITransform::SetWidth(const UIMeasurement& width)
	{
		mWidth = width;
	}

	void UITransform::SetHeight(const UIMeasurement& height)
	{
		mHeight = height;
	}
}