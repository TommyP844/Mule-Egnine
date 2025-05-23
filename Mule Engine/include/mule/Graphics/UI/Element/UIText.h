#pragma once

#include "Graphics/UI/Element/UIElement.h"

namespace Mule
{
	class UIText : public UIElement
	{
	public:
		UIText(const std::string& name = "UI Text");

		void SetText(const std::string& text);
		const std::string& GetText() const { return mText; }

		void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;
		void Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;

		void SetAutoSize(bool autoSize);

		void SetScene(WeakRef<UIScene> scene) override;

	private:
		std::string mText;
		bool mAutoSize;
	};
}