#pragma once

#include "UIElement.h"
#include "UIText.h"
#include "Graphics/UI/Style/UIButtonStyle.h"

namespace Mule
{
	class UIButton : public UIElement<UIButtonStyle>
	{
	public:
		UIButton(const std::string& name = "UI Button");
		virtual ~UIButton();
				
		void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;
		void Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;
		void SetHandle(UIHandle handle) override;
		void SetScene(WeakRef<UIScene> scene) override;

		Ref<UIText> GetTextElement() const { return mButtonText; }
		void SetTextElement(Ref<UIText> textElem) { mButtonText = textElem; }


	private:
		Ref<UIText> mButtonText;
	};
}
