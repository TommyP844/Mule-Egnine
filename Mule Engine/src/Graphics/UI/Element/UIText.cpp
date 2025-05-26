#include "Graphics/UI/Element/UIText.h"

#include "Graphics/Renderer/RenderCommand.h"
#include "Graphics/UI/UIFont.h"
#include "Core/StringUtil.h"

namespace Mule
{
	UIText::UIText(const std::string& name)
		:
		UIElement(name, UIElementType::UIText),
		mText(""),
		mAutoSize(false)
	{
	}

	void UIText::SetText(const std::string& text)
	{
		mText = text;
	}

	void UIText::Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme)
	{
		if (!mVisible)
			return;
		
		WeakRef<UITextStyle> style = mStyle ? mStyle : theme->TextStyle;
		WeakRef<UITextStyle> fallbackStyle = theme->TextStyle;

		AssetHandle fontHandle = style->GetFontHandle(mState, fallbackStyle);

		if (!fontHandle)
			return;

		WeakRef<UIFont> font = assetManager->Get<UIFont>(fontHandle);
		WeakRef<Texture2D> fontAtlas = assetManager->Get<Texture2D>(font->GetAtlasHandle());
		float fontSize = style->GetFontSize(mState, fallbackStyle);
		glm::vec4 fontColor = style->GetForegroundColor(mState, fallbackStyle);
		//glm::vec4 backgroundColor = mStyle->GetValue<glm::vec4>(mState, UIStyleKey::BackgroundColor, theme);
		//bool hasBorder = mStyle->GetValue<bool>(mState, UIStyleKey::HasBorder, theme);
		//glm::vec4 borderColor = mStyle->GetValue<glm::vec4>(mState, UIStyleKey::BorderColor, theme);
		//float borderWidth = mStyle->GetValue<float>(mState, UIStyleKey::BorderWidth, theme);
		//glm::vec2 padding = mStyle->GetValue<glm::vec2>(mState, UIStyleKey::Padding, theme);

		const UIRect& rect = GetScreenRect();

		glm::vec2 cursor = glm::vec2(rect.X, rect.Y + font->GetLineHeight() * fontSize);

		for (auto c : mText)
		{			
			if (c == '\n')
			{
				cursor.x = rect.X;
				cursor.y += font->GetLineHeight() * fontSize;
				continue;
			}
			const auto& glyph = font->GetGlyph(c);

			glm::vec2 min = cursor + glyph.PlaneMin * fontSize;
			glm::vec2 max = cursor + glyph.PlaneMax * fontSize;

			if (max.x > rect.X + rect.Width)
			{
				cursor.x = rect.X;
				cursor.y += font->GetLineHeight() * fontSize;

				min = cursor + glyph.PlaneMin * fontSize;
				max = cursor + glyph.PlaneMax * fontSize;
			}

			if (c == ' ')
			{
				cursor.x += glyph.Advance * fontSize;
				continue;
			}

			DrawCharacterCommand command(
				min,
				max,
				glyph.MinUV,
				glyph.MaxUV,
				fontColor,
				fontAtlas->GetGlobalIndex()
			);


			commandList.AddCommand(command);

			cursor.x += glyph.Advance * fontSize;
		}
		
	}

	void UIText::Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme)
	{
		
		if (mAutoSize)
		{
			WeakRef<UITextStyle> style = mStyle ? mStyle : theme->TextStyle;
			WeakRef<UITextStyle> fallbackStyle = theme->TextStyle;

			float fontSize = style->GetFontSize(mState, fallbackStyle);
			auto fontHandle = style->GetFontHandle(mState, fallbackStyle);
			auto font = assetManager->Get<UIFont>(fontHandle); // Font should always exist, default loaded at engine startup

			if (font)
			{
				glm::vec2 textSize = font->CalculateSize(mText, fontSize, parentRect.Width);

				mTransform.Width = UIMeasurement(textSize.x, UIUnitType::Pixels);
				mTransform.Height = UIMeasurement(textSize.y, UIUnitType::Pixels);
			}
			else
			{
				mTransform.Width = UIMeasurement(10.f, UIUnitType::Pixels);
				mTransform.Height = UIMeasurement(10.f, UIUnitType::Pixels);
			}
		}
		
		UpdateRect(parentRect);		
	}

	void UIText::SetAutoSize(bool autoSize)
	{
		mAutoSize = true;
		mIsDirty = true;
	}

	void UIText::SetScene(WeakRef<UIScene> scene)
	{
		mScene = scene;
	}

	void UIText::SetHandle(UIHandle handle)
	{
		mHandle = handle;
	}
}