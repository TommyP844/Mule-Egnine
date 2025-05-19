#include "Graphics/UI/Element/UIElement.h"

namespace Mule
{
	UIElement::UIElement(const std::string& name)
		:
		mName(name)
	{
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
}