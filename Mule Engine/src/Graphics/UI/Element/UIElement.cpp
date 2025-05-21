#include "Graphics/UI/Element/UIElement.h"

namespace Mule
{
	UIElement::UIElement(const std::string& name, UIElementType elementType)
		:
		mName(name),
		mType(elementType)
	{
		mStyle = nullptr;
	}

	void UIElement::AddChild(const Ref<UIElement>& child)
	{
		if (!child)
			return;

		// Prevent double parenting
		if (child->mParent == this)
			return;

		// Detach from previous parent (optional but safe)
		if (child->mParent)
			child->mParent->RemoveChild(child);

		child->mParent = this;
		mChildren.push_back(child);
	}

	void UIElement::RemoveChild(const Ref<UIElement>& child)
	{
		auto it = std::find(mChildren.begin(), mChildren.end(), child);
		if (it != mChildren.end())
		{
			(*it)->mParent = nullptr;
			mChildren.erase(it);
		}
	}

	void UIElement::Update(const UIRect& parentRect)
	{
		if (!mVisible)
			return;

		if (mIsDirty)
		{
			mScreenRect = mTransform.CalculateRect(parentRect, std::nullopt);
		}

		for (auto child : mChildren)
			child->Update(parentRect);
	}

	WeakRef<UIElement> UIElement::HitTest(float screenX, float screenY)
	{
		if (screenX >= mScreenRect.X && screenX <= (mScreenRect.X + mScreenRect.Width)
			&& screenY >= mScreenRect.Y && screenY <= (mScreenRect.Y + mScreenRect.Height))
			return this;

		for (auto child : mChildren)
		{
			auto element = child->HitTest(screenX, screenY);
			if (element)
				return element;
		}

		return nullptr;
	}
}