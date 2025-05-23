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

		AssetHandle fontHandle = mStyle->GetValue<AssetHandle>(UIStyleKey::Font, theme);

		if (!fontHandle)
			return;

		WeakRef<UIFont> font = assetManager->Get<UIFont>(fontHandle);
		WeakRef<Texture2D> fontAtlas = assetManager->Get<Texture2D>(font->GetAtlasHandle());
		float fontSize = mStyle->GetValue<float>(UIStyleKey::FontSize, theme);
		glm::vec4 fontColor = mStyle->GetValue<glm::vec4>(UIStyleKey::ForegroundColor, theme);
		glm::vec4 backgroundColor = mStyle->GetValue<glm::vec4>(UIStyleKey::BackgroundColor, theme);
		bool hasBorder = mStyle->GetValue<bool>(UIStyleKey::HasBorder, theme);
		glm::vec4 borderColor = mStyle->GetValue<glm::vec4>(UIStyleKey::BorderColor, theme);
		float borderWidth = mStyle->GetValue<float>(UIStyleKey::BorderWidth, theme);
		glm::vec2 padding = mStyle->GetValue<glm::vec2>(UIStyleKey::Padding, theme);

		const UIRect& rect = GetScreenRect();

		glm::vec2 cursor = padding + glm::vec2(rect.X, rect.Y + font->GetLineHeight() * fontSize);

		for (auto c : mText)
		{			
			if (c == '\n')
			{
				cursor.x = rect.X + padding.x;
				cursor.y += font->GetLineHeight() * fontSize;
				continue;
			}
			const auto& glyph = font->GetGlyph(c);

			glm::vec2 min = cursor + glyph.PlaneMin * fontSize;
			glm::vec2 max = cursor + glyph.PlaneMax * fontSize;

			if (max.x > rect.X + rect.Width - padding.x)
			{
				cursor.x = rect.X + padding.x;
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

	// TODO: needs option to decide whether the user can set the size, or the text sets the size
	void UIText::Update(const UIRect& parentRect, WeakRef<AssetManager> assetManager, WeakRef<UITheme> theme)
	{
		if (mAutoSize)
		{
			auto fontHandle = mStyle->GetValue<AssetHandle>(UIStyleKey::Font, theme);
			auto font = assetManager->Get<UIFont>(fontHandle); // Font should always exist, default loaded at engine startup

			glm::vec2 textSize = font->CalculateSize(mText, mStyle, theme, parentRect.Width);

			mTransform.Width = UIMeasurement(textSize.x, UIUnitType::Pixels);
			mTransform.Height = UIMeasurement(textSize.y, UIUnitType::Pixels);
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
}