#include "Graphics/UI/Element/UIText.h"

#include "Graphics/Renderer/RenderCommand.h"

namespace Mule
{
	UIText::UIText(const std::string& name)
		:
		UIElement(name, UIElementType::UIText),
		mFontSize(25.f),
		mText("abc jg")
	{
	}

	void UIText::SetText(const std::string& text)
	{
		mText = text;
	}

	void UIText::Render(CommandList& commandList, const UIRect& parentRect, WeakRef<AssetManager> assetManager)
	{
		if (!mVisible || !mStyle)
			return;

		//glm::vec4 backgroundColor = mStyle->GetValue<glm::vec4>(UIStyleKey::BackgroundColor);
		const UIRect& rect = GetScreenRect();

		//DrawScreenSpaceQuadCommand command(
		//	{ rect.X, rect.Y },				// Screen space position	(pixels)
		//	{ rect.Width, rect.Height },	// Size						(pixels)
		//	glm::vec4(1.f, 0.f, 0.f, 1.f),				// Background Color
		//	false,							// Has Border				(TODO: get from Style)
		//	glm::vec4(0.f),					// Border Color				(TODO: Get from Style)
		//	0.f								// Border Thickness			(TODO: Get from Style)
		//	);
		//commandList.AddCommand(command);

		if (mStyle->HasValue(UIStyleKey::Font))
		{
			AssetHandle fontHandle = mStyle->GetValue<AssetHandle>(UIStyleKey::Font);
			if (fontHandle)
			{
				WeakRef<UIFont> font = assetManager->Get<UIFont>(fontHandle);
				WeakRef<Texture2D> fontAtlas = assetManager->Get<Texture2D>(font->GetAtlasHandle());


				glm::vec2 cursor = { rect.X, rect.Y };
				for (auto c : mText)
				{
					const auto& glyph = font->GetGlyph(c);

					glm::vec2 pos = cursor;

					glm::vec2 min = pos + glyph.PlaneMin * mFontSize;
					glm::vec2 max = pos + glyph.PlaneMax * mFontSize;

					DrawUICharacterCommand command(
						min,
						max,
						glyph.MinUV,
						glyph.MaxUV,
						glm::vec4(1.f),
						fontAtlas->GetGlobalIndex()
					);


					commandList.AddCommand(command);

					cursor.x += glyph.Advance * mFontSize;
				}
			}
		}


		for (auto child : GetChildren())
		{
			child->Render(commandList, rect, assetManager);
		}
	}
}