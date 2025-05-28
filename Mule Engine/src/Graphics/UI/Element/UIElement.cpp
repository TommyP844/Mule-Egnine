#include "Graphics/UI/Element/UIElement.h"

#include "Graphics/UI/UIScene.h"

namespace Mule
{
	UIBaseElement::UIBaseElement(const std::string& name, UIElementType elementType, UIHandle handle)
		:
		mName(name),
		mType(elementType),
		mHandle(handle),
		mIsDirty(true),
		mState(UIElementState::Idle)
	{
	}

	void UIBaseElement::AddAnchor(UIHandle targetElement, UIAnchorAxis targetAxis, UIAnchorAxis selfAxis)
	{
		mAnchors[selfAxis] = {
			targetElement,
			targetAxis,
			selfAxis
		};

		switch (selfAxis) {
		case UIAnchorAxis::Top:
			mTransform.Top.reset();
			RemoveAnchor(UIAnchorAxis::CenterVertical);
			break;
		case UIAnchorAxis::Bottom:
			mTransform.Bottom.reset();
			RemoveAnchor(UIAnchorAxis::CenterVertical);
			break;
		case UIAnchorAxis::Left:
			mTransform.Left.reset();
			RemoveAnchor(UIAnchorAxis::CenterHorizontal);
			break;
		case UIAnchorAxis::Right:
			mTransform.Right.reset();
			RemoveAnchor(UIAnchorAxis::CenterHorizontal);
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

	void UIBaseElement::RemoveAnchor(UIAnchorAxis selfAxis)
	{
		auto iter = mAnchors.find(selfAxis);
		if (iter != mAnchors.end())
			mAnchors.erase(iter);
	}

	void UIBaseElement::RemoveAllAnchors()
	{
		mAnchors.clear();
	}

	bool UIBaseElement::IsAnchoredToElementAxis(UIHandle element, UIAnchorAxis anchorAxis) const
	{
		for (const auto& [axis, anchor] : mAnchors)
			if (anchor.TargetElement == element && anchorAxis == anchor.Target)
				return true;
		return false;
	}

	// TODO:
	void UIBaseElement::OnEvent(WeakRef<Event> event)
	{
	}

	UIRect UIBaseElement::ResolveRect(const UIRect& parentRect)
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

			const UIRect& targetRect = targetElement->GetFinalRect();

			auto getVertical = [](UIAnchorAxis axis, const UIRect& r) -> float {
				switch (axis) {
				case UIAnchorAxis::Top: return r.y;
				case UIAnchorAxis::Bottom: return r.y + r.height;
				case UIAnchorAxis::CenterVertical: return r.y + r.height / 2.0f;
				default: return 0.0f;
				}
				};

			auto getHorizontal = [](UIAnchorAxis axis, const UIRect& r) -> float {
				switch (axis) {
				case UIAnchorAxis::Left: return r.x;
				case UIAnchorAxis::Right: return r.x + r.width;
				case UIAnchorAxis::CenterHorizontal: return r.x + r.width / 2.0f;
				default: return 0.0f;
				}
				};

			switch (selfAxis)
			{
			case UIAnchorAxis::Top:
				mTransform.Top = UIMeasurement(getVertical(anchor.Target, targetRect), UIUnitType::Pixels);
				break;
			case UIAnchorAxis::Bottom:
				mTransform.Bottom = UIMeasurement(parentRect.height - getVertical(anchor.Target, targetRect), UIUnitType::Pixels);
				break;
			case UIAnchorAxis::Left:
				mTransform.Left = UIMeasurement(getHorizontal(anchor.Target, targetRect), UIUnitType::Pixels);
				break;
			case UIAnchorAxis::Right:
				mTransform.Right = UIMeasurement(parentRect.width - getHorizontal(anchor.Target, targetRect), UIUnitType::Pixels);
				break;
			case UIAnchorAxis::CenterHorizontal: {
				float targetX = getHorizontal(anchor.Target, targetRect);
				if (mTransform.Width) {
					float w = mTransform.Width->Resolve(parentRect.width);
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
					float h = mTransform.Height->Resolve(parentRect.height);
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
		l = mTransform.Left ? mTransform.Left->Resolve(parentRect.width) : 0.f;
		r = mTransform.Right ? mTransform.Right->Resolve(parentRect.width) : 0.f;
		t = mTransform.Top ? mTransform.Top->Resolve(parentRect.height) : 0.f;
		b = mTransform.Bottom ? mTransform.Bottom->Resolve(parentRect.height) : 0.f;
		w = mTransform.Width ? mTransform.Width->Resolve(parentRect.width) : 0.f;
		h = mTransform.Height ? mTransform.Height->Resolve(parentRect.height) : 0.f;

		float rx = 0.f, ry = 0.f, rw = 0.f, rh = 0.f;

		// Horizontal Layout
		if (mTransform.Left && mTransform.Width)
		{
			rx = l; // Needs parent.x if no anchors present
			rw = w;
		}
		else if (mTransform.Left && mTransform.Right)
		{
			rx = parentRect.x + l;
			rw = parentRect.width - l - r;
		}
		else if (mTransform.Width && mTransform.Right)
		{
			rw = w;
			rx = parentRect.x + parentRect.width - r - rw;
		}
		else
		{
			rx = parentRect.x;
			rw = parentRect.width;
		}

		// Vertical layout
		if (mTransform.Top && mTransform.Height)
		{
			ry = t;
			rh = h;
		}
		else if (mTransform.Top && mTransform.Bottom)
		{
			ry = parentRect.y + t;
			rh = parentRect.height - t - t;
		}
		else if (mTransform.Height && mTransform.Bottom)
		{
			rh = h;
			ry = parentRect.y + parentRect.height - b - rh;
		}
		else
		{
			ry = parentRect.y;
			rh = parentRect.height;
		}

		UIRect rect = {
			rx, ry,
			rw, rh
		};

		return rect;
	}

	void UIBaseElement::AddChild(Ref<UIBaseElement> child)
	{
		mChildren.push_back(child);
	}

	void UIBaseElement::SetHandle(UIHandle handle)
	{
		for (auto child : mChildren)
		{
			for (auto [selfAxis, anchor] : mAnchors)
			{
				if (anchor.TargetElement == mHandle)
				{
					child->SetHandle(handle);
				}
			}
		}
		mHandle = handle;
	}

	void UIBaseElement::SetScene(WeakRef<UIScene> scene)
	{
		mScene = scene;
		for (auto child : mChildren)
		{
			child->SetScene(scene);
		}
	}

	WeakRef<UIBaseElement> UIBaseElement::HitTest(float screenX, float screenY)
	{
		if (screenX >= mFinalRect.x && screenX <= (mFinalRect.x + mFinalRect.width)
			&& screenY >= mFinalRect.y && screenY <= (mFinalRect.y + mFinalRect.height))
			return this;

		return nullptr;
	}
	
	void UIBaseElement::SetLeft(float value, UIUnitType type)
	{
		RemoveAnchor(UIAnchorAxis::CenterHorizontal);
		RemoveAnchor(UIAnchorAxis::Left);
		mTransform.Left = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIBaseElement::SetRight(float value, UIUnitType type)
	{
		RemoveAnchor(UIAnchorAxis::CenterHorizontal);
		RemoveAnchor(UIAnchorAxis::Right);
		mTransform.Right = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIBaseElement::SetTop(float value, UIUnitType type)
	{
		RemoveAnchor(UIAnchorAxis::CenterVertical);
		RemoveAnchor(UIAnchorAxis::Top);
		mTransform.Top = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIBaseElement::SetBottom(float value, UIUnitType type)
	{
		RemoveAnchor(UIAnchorAxis::CenterVertical);
		RemoveAnchor(UIAnchorAxis::Bottom);
		mTransform.Bottom = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIBaseElement::SetWidth(float value, UIUnitType type)
	{
		// TODO: remove anchors if need be
		mTransform.Width = UIMeasurement(value, type);
		mIsDirty = true;
	}

	void UIBaseElement::SetHeight(float value, UIUnitType type)
	{
		// TODO: remove anchors if need be
		mTransform.Height = UIMeasurement(value, type);
		mIsDirty = true;
	}
	
}