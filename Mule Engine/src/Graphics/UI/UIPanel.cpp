#include "Graphics/UI/UIPanel.h"

namespace Mule
{
	UIPanel::UIPanel(const std::string& name)
		:
		mName(name)
	{
	}

	void UIPanel::AddElement(Ref<UIElement> element)
	{
		auto it = std::find(mElements.begin(), mElements.end(), element);
		if (it == mElements.end())
		{
			mElements.push_back(element);
		}
	}

	void UIPanel::RemoveElement(Ref<UIElement> element)
	{
		auto it = std::find(mElements.begin(), mElements.end(), element);
		if (it != mElements.end())
		{
			mElements.erase(it);
		}
	}

	void UIPanel::Render(CommandList& commandList, const UIRect& windowRect)
	{
		if (!mVisible)
			return;

		for (auto element : mElements)
		{
			//element->Render(commandList, windowRect);
		}
	}
}