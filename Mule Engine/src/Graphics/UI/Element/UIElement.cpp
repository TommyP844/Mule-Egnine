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
				mTransform.Top = UIMeasurement(getVertical(anchor.Target, targetRect), UIUnitType::Pixels);
				break;
			case UIAnchorAxis::Bottom:
				mTransform.Bottom = UIMeasurement(parentRect.Height - getVertical(anchor.Target, targetRect), UIUnitType::Pixels);
				break;
			case UIAnchorAxis::Left:
				mTransform.Left = UIMeasurement(getHorizontal(anchor.Target, targetRect), UIUnitType::Pixels);
				break;
			case UIAnchorAxis::Right:
				mTransform.Right = UIMeasurement(parentRect.Width - getHorizontal(anchor.Target, targetRect), UIUnitType::Pixels);
				break;
			case UIAnchorAxis::CenterHorizontal: {
				float targetX = getHorizontal(anchor.Target, targetRect);
				if (mTransform.Width) {
					float w = mTransform.Width->Resolve(parentRect.Width);
					mTransform.Left = UIMeasurement(targetX - w / 2.0f, UIUnitType::Pixels);
					mTransform.Width = UIMeasurement(w, UIUnitType::Pixels);
				}
				else {
					mTransform.Left = UIMeasurement(targetX, UIUnitType::Pixels);
				}
				break;
			}
			case UIAnchorAxis::CenterVertical: {
				float targetY = getVertical(anchor.Target, targetRect);
				if (mTransform.Height) {
					float h = mTransform.Height->Resolve(parentRect.Height);
					mTransform.Top = UIMeasurement(targetY - h / 2.0f, UIUnitType::Pixels);
					mTransform.Height = UIMeasurement(h, UIUnitType::Pixels);
				}
				else {
					mTransform.Top = UIMeasurement(targetY, UIUnitType::Pixels);
				}
				break;
			}
			default:
				break;
			}
		}

		for (auto axis : anchorsToRemove)
			mAnchors.erase(axis);

		float l = 0.f, r = 0.f, t = 0.f, b = 0.f, w = 0.f, h = 0.f;
		l = mTransform.Left ? mTransform.Left->Resolve(parentRect.Width) : 0.f;
		r = mTransform.Right ? mTransform.Right->Resolve(parentRect.Width) : 0.f;
		t = mTransform.Top ? mTransform.Top->Resolve(parentRect.Height) : 0.f;
		b = mTransform.Bottom ? mTransform.Bottom->Resolve(parentRect.Height) : 0.f;
		w = mTransform.Width ? mTransform.Width->Resolve(parentRect.Width) : 0.f;
		h = mTransform.Height ? mTransform.Height->Resolve(parentRect.Height) : 0.f;

		float rx = 0.f, ry = 0.f, rw = 0.f, rh = 0.f;

		// Horizontal Layout
		if (mTransform.Left && mTransform.Width)
		{
			rx = l; // Needs parent.x if no anchors present
			rw = w;
		}
		else if (mTransform.Left && mTransform.Right)
		{
			rx = parentRect.X + l;
			rw = parentRect.Width - l - r;
		}
		else if (mTransform.Width && mTransform.Right)
		{
			rw = w;
			rx = parentRect.X + parentRect.Width - r - rw;
		}
		else
		{
			rx = parentRect.X;
			rw = parentRect.Width;
		}

		// Vertical layout
		if (mTransform.Top && mTransform.Height)
		{
			ry = t;
			rh = h;
		}
		else if (mTransform.Top && mTransform.Bottom)
		{
			ry = parentRect.Y + t;
			rh = parentRect.Height - t - t;
		}
		else if (mTransform.Height && mTransform.Bottom)
		{
			rh = h;
			ry = parentRect.Y + parentRect.Height - b - rh;
		}
		else
		{
			ry = parentRect.Y;
			rh = parentRect.Height;
		}

		mScreenRect = {
			rx, ry,
			rw, rh
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