#include "Graphics/UI/Element/UIText.h"

#include "Graphics/Renderer/RenderCommand.h"
#include "Graphics/UI/UIFont.h"
#include "Core/StringUtil.h"

#include "Graphics/UI/UIScene.h"

namespace Mule
{
	UIText::UIText(const std::string& name)
		:
		UIElement(name, UIElementType::UIText),
		mText("")
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

		if (!font)
			return;

		WeakRef<Texture2D> fontAtlas = assetManager->Get<Texture2D>(font->GetAtlasHandle());
		float fontSize = style->GetFontSize(mState, fallbackStyle);
		glm::vec4 fontColor = style->GetFontColor(mState, fallbackStyle);
		//glm::vec4 backgroundColor = mStyle->GetValue<glm::vec4>(mState, UIStyleKey::BackgroundColor, theme);
		//bool hasBorder = mStyle->GetValue<bool>(mState, UIStyleKey::HasBorder, theme);
		//glm::vec4 borderColor = mStyle->GetValue<glm::vec4>(mState, UIStyleKey::BorderColor, theme);
		//float borderWidth = mStyle->GetValue<float>(mState, UIStyleKey::BorderWidth, theme);
		//glm::vec2 padding = mStyle->GetValue<glm::vec2>(mState, UIStyleKey::Padding, theme);

		const UIRect& rect = GetContentRect();

		glm::vec2 cursor = glm::vec2(rect.x, rect.y + font->GetLineHeight() * fontSize);

		for (auto c : mText)
		{			
			if (c == '\n')
			{
				cursor.x = rect.x;
				cursor.y += font->GetLineHeight() * fontSize;
				continue;
			}
			const auto& glyph = font->GetGlyph(c);

			glm::vec2 min = cursor + glyph.PlaneMin * fontSize;
			glm::vec2 max = cursor + glyph.PlaneMax * fontSize;

			// Need epsilon for floating point precision issues
			float espsilon = 1.f / fontSize;
			if (max.x > rect.x + rect.width + espsilon)
			{
				cursor.x = rect.x;
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

	void UIText::Measure(const UIRect& parentRect)
	{
		auto assetManager = mScene->GetAssetManager();
		WeakRef<UITextStyle> style = mStyle ? mStyle : mScene->GetTheme()->TextStyle;
		WeakRef<UITextStyle> fallbackStyle = mScene->GetTheme()->TextStyle;

		AssetHandle fontHandle = style->GetFontHandle(mState, fallbackStyle);
		if (!fontHandle)
			return;

		WeakRef<UIFont> font = assetManager->Get<UIFont>(fontHandle);
		if (!font)
			return;

		float wrapWidth = parentRect.width;

		if (mTransform.Width.has_value() && mTransform.Width->GetUnitType() != UIUnitType::Auto)
		{
			wrapWidth = mTransform.Width->Resolve(parentRect.width);
		}

		float fontSize = style->GetFontSize(mState, fallbackStyle);
		glm::vec2 size = font->CalculateSize(mText, fontSize, parentRect.width);

		mMeasuredSize = size;
	}

	void UIText::Layout(const UIRect& parentRect)
	{
		WeakRef<UITextStyle> style = mStyle ? mStyle : mScene->GetTheme()->TextStyle;
		WeakRef<UITextStyle> fallbackStyle = mScene->GetTheme()->TextStyle;

		const UIPadding& padding = style->GetPadding(mState, fallbackStyle);

		UIRect rect = ResolveRect(parentRect);

		if (mTransform.Width.has_value() && mTransform.Width->GetUnitType() == UIUnitType::Auto)
		{
			rect.width = mMeasuredSize.x + padding.Left.Resolve(rect.width) + padding.Right.Resolve(rect.width);
		}
		if (mTransform.Height.has_value() && mTransform.Height->GetUnitType() == UIUnitType::Auto)
		{
			rect.height = mMeasuredSize.y + padding.Top.Resolve(rect.height) + padding.Bottom.Resolve(rect.height);
		}

		mFinalRect = rect;
		mContentRect = rect;

		mContentRect.x += padding.Left.Resolve(rect.width);
		mContentRect.y += padding.Top.Resolve(rect.height);
		mContentRect.width -= padding.Left.Resolve(rect.width) + padding.Right.Resolve(rect.width);
		mContentRect.height -= padding.Top.Resolve(rect.height) + padding.Bottom.Resolve(rect.height);

		for (auto child : mChildren)
		{
			child->Measure(mContentRect);
			child->Layout(mContentRect);
		}
	}
}