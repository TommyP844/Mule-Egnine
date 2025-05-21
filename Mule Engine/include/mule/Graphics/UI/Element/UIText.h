#pragma once

#include "Graphics/UI/Element/UIElement.h"

namespace Mule
{
	class UIText : public UIElement
	{
	public:
		UIText(const std::string& name);

		void SetText(const std::string& text);
		const std::string& GetText() const { return mText; }

		void Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme) override;

	private:
		std::string mText;
	};
}