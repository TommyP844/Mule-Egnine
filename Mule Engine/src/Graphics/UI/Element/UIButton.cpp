#include "Graphics/UI/Element/UIButton.h"

#include "Graphics/UI/UIFont.h"

namespace Mule
{
	UIButton::UIButton(const std::string& name)
		:
		UIElement(name, UIElementType::UIButton)
	{
	}

	UIButton::~UIButton()
	{
	}
	
	void UIButton::Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme)
	{
		if (!mVisible)
			return;

	}

	void UIButton::Measure(const UIRect& parentRect)
	{
	}

	void UIButton::Layout(const UIRect& parentRect)
	{
	}

}