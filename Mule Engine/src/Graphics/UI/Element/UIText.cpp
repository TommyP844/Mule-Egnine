#include "Graphics/UI/Element/UIText.h"

#include "Graphics/Renderer/RenderCommand.h"

namespace Mule
{
	UIText::UIText(const std::string& name)
		:
		UIElement(name)
	{
	}

	void UIText::SetText(const std::string& text)
	{
		mText = text;
	}

	void UIText::Render(CommandList& commandList, const UIRect& parentRect)
	{
		if (!mVisible)
			return;

		UIRect rect = mTransform.CalculateRect(parentRect, std::nullopt);

		glm::vec4 backgroundColor = mStyle->GetValue<glm::vec4>(UIStyleKey::BackgroundColor);

		DrawScreenSpaceQuadCommand command(
			{ rect.X, rect.Y },				// Screen space position	(pixels)
			{ rect.Width, rect.Height },	// Size						(pixels)
			backgroundColor,				// Background Color
			false,							// Has Border				(TODO: get from Style)
			glm::vec4(0.f),					// Border Color				(TODO: Get from Style)
			0.f								// Border Thickness			(TODO: Get from Style)
			);

		commandList.AddCommand(command);

		for (auto child : GetChildren())
		{
			child->Render(commandList, rect);
		}
	}
}