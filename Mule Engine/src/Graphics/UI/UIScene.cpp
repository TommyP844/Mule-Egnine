#include "Graphics/UI/UIScene.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	UIScene::UIScene(const std::string name)
		:
		Asset(name)
	{
	}

	void UIScene::Render(CommandList& commandList, const UIRect& screenRect)
	{
		for (auto panel : mPanels)
			panel->Render(commandList, screenRect);

		for (auto element : mElements)
			element->Render(commandList, screenRect);
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
}