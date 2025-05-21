#include "Graphics/UI/UIScene.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	UIScene::UIScene(const std::string name)
		:
		Asset(name)
	{
	}

	void UIScene::Render(CommandList& commandList, const UIRect& screenRect, WeakRef<AssetManager> assetManager)
	{
		auto theme = assetManager->Get<UITheme>(mThemeHandle);
		theme = theme ? theme : UITheme::GetDefault();

		for (auto panel : mPanels)
			panel->Render(commandList, screenRect);

		for (auto element : mElements)
			element->Render(commandList, screenRect, assetManager, theme);
	}

	void UIScene::AddUIElement(Ref<UIElement> element)
	{
		auto iter = std::find(mElements.begin(), mElements.end(), element);
		if (iter != mElements.end())
		{
			SPDLOG_WARN("Element already exists in UIScene: {}", element->GetName());
			return;
		}
		mElements.push_back(element);
		mElementHandles[element->GetHandle()] = element;
	}

	void UIScene::RemoveUIElement(Ref<UIElement> element)
	{
		auto iter = std::find(mElements.begin(), mElements.end(), element);
		if (iter == mElements.end())
		{
			SPDLOG_WARN("Element does not exist in UIScene: {}", element->GetName());
			return;
		}
		mElements.erase(iter);
		mElementHandles.erase(element->GetHandle());
	}

	void UIScene::AddUIPanel(Ref<UIPanel> panel)
	{
		auto iter = std::find(mPanels.begin(), mPanels.end(), panel);
		if (iter != mPanels.end())
		{
			SPDLOG_WARN("Element already exists in UIScene: {}", panel->GetName());
			return;
		}
		mPanels.push_back(panel);
	}

	void UIScene::RemoveUIPanel(Ref<UIPanel> panel)
	{
		auto iter = std::find(mPanels.begin(), mPanels.end(), panel);
		if (iter == mPanels.end())
		{
			SPDLOG_WARN("UIPanel does not exist in UIScene: {}", panel->GetName());
			return;
		}
		mPanels.push_back(panel);
	}

	void UIScene::Update(const UIRect& windowRect)
	{
		for (auto element : mElements)
			element->Update(windowRect);
	}

	WeakRef<UIElement> UIScene::HitTest(float screenX, float screenY)
	{
		for (auto element : mElements)
		{
			auto found = element->HitTest(screenX, screenY);
			if (found)
				return found;
		}

		return nullptr;
	}
	WeakRef<UIElement> UIScene::GetElement(UIHandle handle) const
	{
		auto iter = mElementHandles.find(handle);
		if (iter != mElementHandles.end())
			return iter->second;
		return nullptr;
	}
}