#include "Graphics/UI/Element/UIElement.h"

#include "Graphics/UI/UIScene.h"

namespace Mule
{
	UIElement::UIElement(const std::string& name, UIElementType elementType, UIHandle handle)
		:
		mName(name),
		mType(elementType),
		mHandle(handle),
		mIsDirty(true)
	{
		mStyle = UIStyle::GetDefault();
	}

	void UIElement::AddAnchor(UIHandle targetElement, UIAnchorAxis targetAxis, UIAnchorAxis selfAxis)
	{
		mAnchors[selfAxis] = {
			targetElement,
			targetAxis,
			selfAxis
		};

		switch (selfAxis) {
		case UIAnchorAxis::Top:
			mTransform.Top.reset();
			break;
		case UIAnchorAxis::Bottom:
			mTransform.Bottom.reset();
			break;
		case UIAnchorAxis::Left:
			mTransform.Left.reset();
			break;
		case UIAnchorAxis::Right:
			mTransform.Right.reset();
			break;
		case UIAnchorAxis::CenterHorizontal:
			mTransform.Left.reset();
			mTransform.Right.reset();
			break;
		case UIAnchorAxis::CenterVertical:
			mTransform.Top.reset();
			mTransform.Bottom.reset();
			break;
		default:
			break;
		}
	}

	void UIElement::RemoveAnchor(UIAnchorAxis selfAxis)
	{
		auto iter = mAnchors.find(selfAxis);
		if (iter != mAnchors.end())
			mAnchors.erase(iter);
	}

	void UIElement::RemoveAllAnchors()
	{
		mAnchors.clear();
	}

	bool UIElement::IsAnchoredToElementAxis(UIHandle element, UIAnchorAxis anchorAxis) const
	{
		for (const auto& [axis, anchor] : mAnchors)
			if (anchor.TargetElement == element && anchorAxis == anchor.Target)
				return true;
		return false;
	}

	void UIElement::UpdateRect(const UIRect& parentRect)
	{
		std::optional<float> top, left, bottom, right, width, height;

		std::vector<UIAnchorAxis> anchorsToRemove;
		for (const auto& [selfAxis, anchor] : mAnchors)
		{
			auto targetElement = mScene->GetElement(anchor.TargetElement);

			if (!targetElement)
			{
				anchorsToRemove.push_back(selfAxis);
				continue;
			}

			const UIRect& targetRect = targetElement->GetScreenRect();


			auto getVertical = [](UIAnchorAxis axis, const UIRect& r) -> float {
				switch (axis) {
				case UIAnchorAxis::Top: return r.Y;
				case UIAnchorAxis::Bottom: return r.Y + r.Height;
				case UIAnchorAxis::CenterVertical: return r.Y + r.Height / 2.0f;
				default: return 0.0f;
				}
				};

			auto getHorizontal = [](UIAnchorAxis axis, const UIRect& r) -> float {
				switch (axis) {
				case UIAnchorAxis::Left: return r.X;
				case UIAnchorAxis::Right: return r.X + r.Width;
				case UIAnchorAxis::CenterHorizontal: return r.X + r.Width / 2.0f;
				default: return 0.0f;
				}
				};

			switch (selfAxis)
			{
			case UIAnchorAxis::Top:
				top = getVertical(anchor.Target, targetRect);
				break;
			case UIAnchorAxis::Bottom:
				bottom = parentRect.Height - getVertical(anchor.Target, targetRect);
				break;
			case UIAnchorAxis::Left:
				left = getHorizontal(anchor.Target, targetRect);
				break;
			case UIAnchorAxis::Right:
				right = parentRect.Width - getHorizontal(anchor.Target, targetRect);
				break;
			case UIAnchorAxis::CenterHorizontal: {
				float targetX = getHorizontal(anchor.Target, targetRect);
				if (mTransform.Width) {
					float w = mTransform.Width->Resolve(parentRect.Width);
					left = targetX - w / 2.0f;
					width = w;
				}
				else {
					left = targetX;
				}
				break;
			}
			case UIAnchorAxis::CenterVertical: {
				float targetY = getVertical(anchor.Target, targetRect);
				if (mTransform.Height) {
					float h = mTransform.Height->Resolve(parentRect.Height);
					top = targetY - h / 2.0f;
					height = h;
				}
				else {
					top = targetY;
				}
				break;
			}
			default:
				break;
			}
		}

		for (auto axis : anchorsToRemove)
			mAnchors.erase(axis);

		// TODO: need to check here first and see if we have enough information to generate a rect
		// otherwise we end up with a double offset

		if (!top) top = mTransform.Top ? std::make_optional<float>(mTransform.Top->Resolve(parentRect.Height)) : std::nullopt;
		if (!bottom) bottom = mTransform.Bottom ? std::make_optional<float>(mTransform.Bottom->Resolve(parentRect.Height)) : std::nullopt;
		if (!left) left = mTransform.Left ? std::make_optional<float>(mTransform.Left->Resolve(parentRect.Width)) : std::nullopt;
		if (!right) right = mTransform.Right ? std::make_optional<float>(mTransform.Right->Resolve(parentRect.Width)) : std::nullopt;
		if (!width && (!left || !right)) width = mTransform.Width ? std::make_optional<float>(mTransform.Width->Resolve(parentRect.Width)) : std::nullopt;
		if (!height && (!top || !bottom)) height = mTransform.Height ? std::make_optional<float>(mTransform.Height->Resolve(parentRect.Height)) : std::nullopt;

		float x = 0.f, y = 0.f, w = 0.f, h = 0.f;

		// Horizontal Layout
		if (left && width)
		{
			x = *left; // Needs parent.x if no anchors present
			w = *width;
		}
		else if (left && right)
		{
			x = parentRect.X + *left;
			w = parentRect.Width - *left - *right;
		}
		else if (width && right)
		{
			w = *width;
			x = parentRect.X + parentRect.Width - *right - w;
		}
		else
		{
			x = parentRect.X;
			w = parentRect.Width;
		}

		// Vertical layout
		if (top && height)
		{
			y = *top;
			h = *height;
		}
		else if (top && bottom)
		{
			y = parentRect.Y + *top;
			h = parentRect.Height - *top - *bottom;
		}
		else if (height && bottom)
		{
			h = *height;
			y = parentRect.Y + parentRect.Height - *bottom - h;
		}
		else
		{
			y = parentRect.Y;
			h = parentRect.Height;
		}

		mScreenRect = {
			x, y,
			w, h
		};
	}

	WeakRef<UIElement> UIElement::HitTest(float screenX, float screenY)
	{
		if (screenX >= mScreenRect.X && screenX <= (mScreenRect.X + mScreenRect.Width)
			&& screenY >= mScreenRect.Y && screenY <= (mScreenRect.Y + mScreenRect.Height))
			return this;

		return nullptr;
	}
	
	void UIElement::SetLeft(float value, UIUnitType type)
	{
		// TODO: remove anchors if need be
		mTransform.Left = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIElement::SetRight(float value, UIUnitType type)
	{
		// TODO: remove anchors if need be
		mTransform.Right = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIElement::SetTop(float value, UIUnitType type)
	{
		// TODO: remove anchors if need be
		mTransform.Top = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIElement::SetBottom(float value, UIUnitType type)
	{
		// TODO: remove anchors if need be
		mTransform.Bottom = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIElement::SetWidth(float value, UIUnitType type)
	{
		// TODO: remove anchors if need be
		mTransform.Width = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIElement::SetHeight(float value, UIUnitType type)
	{
		// TODO: remove anchors if need be
		mTransform.Height = UIMeasurement(value, type);
		mIsDirty = true;
	}
	
}