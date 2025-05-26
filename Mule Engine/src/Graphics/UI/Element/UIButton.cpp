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

		WeakRef<UIButtonStyle> style = mStyle ? mStyle : theme->ButtonStyle;
		WeakRef<UIButtonStyle> fallbackStyle = theme->ButtonStyle;
		
		glm::vec4 backgroundColor = style->GetBackgroundColor(mState, fallbackStyle);
		//bool hasBorder = mStyle->GetValue<bool>(mState, UIStyleKey::HasBorder, theme);
		//glm::vec4 borderColor = mStyle->GetValue<glm::vec4>(mState, UIStyleKey::BorderColor, theme);
		//float borderWidth = mStyle->GetValue<float>(mState, UIStyleKey::BorderWidth, theme);
		//glm::vec2 padding = mStyle->GetValue<glm::vec2>(mState, UIStyleKey::Padding, theme);

		const UIRect& rect = GetScreenRect();

		DrawRectCommand command(
			{ rect.X, rect.Y },				// Screen space position	(pixels)
			{ rect.Width, rect.Height },	// Size						(pixels)
			backgroundColor,				// Background Color
			false,						// Has Border				
			glm::vec4(0.f),					// Border Color				
			0.f						// Border Thickness	
		);
		commandList.AddCommand(command);

		mButtonText->Render(commandList, rect, assetManager, theme);
		
	}

	void UIButton::Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme)
	{
		UpdateRect(parentRect);

		mButtonText->Update(mScreenRect, assetManager, theme);
	}

	void UIButton::SetHandle(UIHandle handle)
	{
		mHandle = handle;
		mButtonText->AddAnchor(GetHandle(), UIAnchorAxis::CenterHorizontal, UIAnchorAxis::CenterHorizontal);
		mButtonText->AddAnchor(GetHandle(), UIAnchorAxis::CenterVertical, UIAnchorAxis::CenterVertical);
	}
	
	void UIButton::SetScene(WeakRef<UIScene> scene)
	{
		mScene = scene;
		mButtonText->SetScene(scene);
	}
}