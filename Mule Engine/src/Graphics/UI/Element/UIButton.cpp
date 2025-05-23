#include "Graphics/UI/Element/UIButton.h"

namespace Mule
{
	UIButton::UIButton(const std::string& name)
		:
		UIElement(name, UIElementType::UIButton)
	{
		mButtonText = MakeRef<UIText>();
		mButtonText->SetText("Button");
		mButtonText->AddAnchor(GetHandle(), UIAnchorAxis::CenterHorizontal, UIAnchorAxis::CenterHorizontal);
		mButtonText->AddAnchor(GetHandle(), UIAnchorAxis::CenterVertical, UIAnchorAxis::CenterVertical);
		mButtonText->SetWidth(50, UIUnitType::Pixels);
		mButtonText->SetHeight(50, UIUnitType::Pixels);
		mButtonText->SetAutoSize(true);
	}

	UIButton::~UIButton()
	{
	}
	
	void UIButton::Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme)
	{
		if (!mVisible)
			return;
		
		glm::vec4 backgroundColor = mStyle->GetValue<glm::vec4>(UIStyleKey::BackgroundColor, theme);
		bool hasBorder = mStyle->GetValue<bool>(UIStyleKey::HasBorder, theme);
		glm::vec4 borderColor = mStyle->GetValue<glm::vec4>(UIStyleKey::BorderColor, theme);
		float borderWidth = mStyle->GetValue<float>(UIStyleKey::BorderWidth, theme);
		glm::vec2 padding = mStyle->GetValue<glm::vec2>(UIStyleKey::Padding, theme);

		const UIRect& rect = GetScreenRect();

		DrawRectCommand command(
			{ rect.X, rect.Y },				// Screen space position	(pixels)
			{ rect.Width, rect.Height },	// Size						(pixels)
			backgroundColor,				// Background Color
			hasBorder,						// Has Border				
			borderColor,					// Border Color				
			borderWidth						// Border Thickness	
		);
		commandList.AddCommand(command);

		mButtonText->Render(commandList, rect, assetManager, theme);
	}

	void UIButton::Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme)
	{
		UpdateRect(parentRect);

		mButtonText->Update(mScreenRect, assetManager, theme);
	}
	
	void UIButton::SetScene(WeakRef<UIScene> scene)
	{
		mScene = scene;
		mButtonText->SetScene(scene);
	}
}