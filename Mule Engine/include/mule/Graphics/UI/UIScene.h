#pragma once

#include "UIPanel.h"
#include "Element/UIElement.h"

#include <unordered_map>

namespace Mule
{
	class UIScene : public Asset<AssetType::UIScene>
	{
	public:
		UIScene(const std::string name = "UI Scene");

		void Render(CommandList& commandList, const UIRect& screenRect, WeakRef<AssetManager> assetManager);

		void AddUIElement(Ref<UIElement> element);
		void RemoveUIElement(Ref<UIElement> element);

		void AddUIPanel(Ref<UIPanel> panel);
		void RemoveUIPanel(Ref<UIPanel> panel);

		const std::vector<Ref<UIElement>>& GetUIElements() const { return mElements; }
		const std::vector<Ref<UIPanel>>& GetUIPanels() const { return mPanels; }

		void Update(const UIRect& windowRect);
		WeakRef<UIElement> HitTest(float screenX, float screenY);

		WeakRef<UIElement> GetElement(UIHandle handle) const;

		void SetThemeHandle(AssetHandle themeHandle) { mThemeHandle = themeHandle; }
		AssetHandle GetThemeHandle() const { return mThemeHandle; }

	private:
		std::vector<Ref<UIElement>> mElements;
		std::vector<Ref<UIPanel>> mPanels;
		std::unordered_map<UIHandle, Ref<UIElement>> mElementHandles;
		AssetHandle mThemeHandle;
	};
}