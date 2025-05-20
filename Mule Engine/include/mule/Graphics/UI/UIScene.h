#pragma once

#include "UIPanel.h"
#include "Element/UIElement.h"

namespace Mule
{
	class UIScene : Asset<AssetType::UIScene>
	{
	public:
		UIScene(const std::string name = "UI Scene");

		void Render(CommandList& commandList, const UIRect& screenRect);

		void AddUIElement(Ref<UIElement> element);
		void RemoveUIElement(Ref<UIElement> element);

		void AddUIPanel(Ref<UIPanel> panel);
		void RemoveUIPanel(Ref<UIPanel> panel);

		const std::vector<Ref<UIElement>>& GetUIElements() const { return mElements; }
		const std::vector<Ref<UIPanel>>& GetUIPanels() const { return mPanels; }

		void Update(const UIRect& windowRect);
		WeakRef<UIElement> HitTest(float screenX, float screenY);

	private:
		std::vector<Ref<UIElement>> mElements;
		std::vector<Ref<UIPanel>> mPanels;

	};
}