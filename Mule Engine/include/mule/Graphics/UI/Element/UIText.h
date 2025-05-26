#pragma once

#include "Graphics/UI/Element/UIElement.h"
#include "Graphics/UI/Style/UITextStyle.h"

namespace Mule
{
	class UIText : public UIElement<UITextStyle>
	{
	public:
		UIText(const std::string& name = "UI Text");

		void SetText(const std::string& text);
		const std::string& GetText() const { return mText; }

		void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;
		void Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;
		void SetScene(WeakRef<UIScene> scene) override;
		void SetHandle(UIHandle handle) override;

		void SetAutoSize(bool autoSize);


	private:
		std::string mText;
		bool mAutoSize;
	};
}