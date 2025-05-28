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

		void AddUIElement(Ref<UIBaseElement> element);
		void RemoveUIElement(WeakRef<UIBaseElement> element);

		const std::vector<Ref<UIBaseElement>>& GetUIElements() const { return mElements; }

		void Update(const UIRect& windowRect, WeakRef<AssetManager> assetManager);
		WeakRef<UIBaseElement> HitTest(float screenX, float screenY);

		WeakRef<UIBaseElement> GetElement(UIHandle handle) const;

		void SetThemeHandle(AssetHandle themeHandle) { mThemeHandle = themeHandle; }
		AssetHandle GetThemeHandle() const { return mThemeHandle; }

		void SetAssetManager(WeakRef<AssetManager> assetManager)
		{
			mAssetManager = assetManager;
		}

		WeakRef<AssetManager> GetAssetManager() const
		{
			return mAssetManager;
		}

		WeakRef<UITheme> GetTheme() const { return mTheme; }

	private:
		std::vector<Ref<UIBaseElement>> mElements;
		std::unordered_map<UIHandle, Ref<UIBaseElement>> mElementHandles;
		AssetHandle mThemeHandle;
		WeakRef<UITheme> mTheme;
		WeakRef<AssetManager> mAssetManager;
	};
}