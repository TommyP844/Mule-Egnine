#pragma once

#include "UIElement.h"
#include "UIText.h"

namespace Mule
{
	class UIButton : public UIElement
	{
	public:
		UIButton(const std::string& name = "UI Button");
		virtual ~UIButton();
				
		void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;
		void Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;

		Ref<UIText> GetTextElement() const { return mButtonText; }
		void SetTextElement(Ref<UIText> textElem) { mButtonText = textElem; }

	private:
		Ref<UIText> mButtonText;
	};
}
