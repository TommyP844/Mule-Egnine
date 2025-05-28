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

		for (auto element : mElements)
			element->Render(commandList, screenRect, assetManager, theme);
	}

	void UIScene::AddUIElement(Ref<UIBaseElement> element)
	{
		auto iter = std::find(mElements.begin(), mElements.end(), element);
		if (iter != mElements.end())
		{
			SPDLOG_WARN("Element already exists in UIScene: {}", element->GetName());
			return;
		}
		mElements.push_back(element);
		mElementHandles[element->GetHandle()] = element;
		element->SetScene(this);
	}

	void UIScene::RemoveUIElement(WeakRef<UIBaseElement> element)
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


	void UIScene::Update(const UIRect& windowRect, WeakRef<AssetManager> assetManager)
	{
		mTheme = assetManager->Get<UITheme>(mThemeHandle);
		mTheme = mTheme ? mTheme : UITheme::GetDefault();

		for (auto element : mElements)
		{
			element->Measure(windowRect);
			element->Layout(windowRect);
		}
	}

	WeakRef<UIBaseElement> UIScene::HitTest(float screenX, float screenY)
	{
		for (auto element : mElements)
		{
			auto found = element->HitTest(screenX, screenY);
			if (found)
				return found;
		}

		return nullptr;
	}
	WeakRef<UIBaseElement> UIScene::GetElement(UIHandle handle) const
	{
		auto iter = mElementHandles.find(handle);
		if (iter != mElementHandles.end())
			return iter->second;
		return nullptr;
	}
}