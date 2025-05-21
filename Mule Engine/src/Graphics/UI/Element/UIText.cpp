#include "Graphics/UI/Element/UIText.h"

#include "Graphics/Renderer/RenderCommand.h"
#include "Graphics/UI/UIFont.h"

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
		if (!mVisible || !mStyle)
			return;

		AssetHandle fontHandle = mStyle->GetValue<AssetHandle>(UIStyleKey::Font, theme);

		if (!fontHandle)
			return;

		WeakRef<UIFont> font = assetManager->Get<UIFont>(fontHandle);
		WeakRef<Texture2D> fontAtlas = assetManager->Get<Texture2D>(font->GetAtlasHandle());
		float fontSize = mStyle->GetValue<float>(UIStyleKey::FontSize, theme);
		glm::vec4 fontColor = mStyle->GetValue<glm::vec4>(UIStyleKey::ForegroundColor, theme);
		glm::vec4 backgroundColor = mStyle->GetValue<glm::vec4>(UIStyleKey::BackgroundColor, theme);

		const UIRect& rect = GetScreenRect();

		DrawScreenSpaceQuadCommand command(
			{ rect.X, rect.Y },				// Screen space position	(pixels)
			{ rect.Width, rect.Height },	// Size						(pixels)
			backgroundColor,				// Background Color
			false,							// Has Border				(TODO: get from Style)
			glm::vec4(0.f),					// Border Color				(TODO: Get from Style)
			0.f								// Border Thickness			(TODO: Get from Style)
			);
		commandList.AddCommand(command);

		glm::vec2 cursor = { rect.X, rect.Y + font->GetLineHeight() * fontSize };
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
				if (rect.X == cursor.x && cursor.y > rect.Y + font->GetLineHeight() * fontSize)
				{}
				else
					cursor.x += glyph.Advance * fontSize;

				continue;
			}

			DrawUICharacterCommand command(
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

		for (auto child : GetChildren())
		{
			child->Render(commandList, rect, assetManager, theme);
		}
	}
}