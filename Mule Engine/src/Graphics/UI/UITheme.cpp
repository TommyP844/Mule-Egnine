#include "Graphics/UI/UITheme.h"

#include "Engine Context/EngineAssets.h"

namespace Mule
{
	UITheme::UITheme()
		:
		Asset()
	{
		// Button
		{
			ButtonStyle = MakeRef<UIButtonStyle>();
			auto& idleVars = ButtonStyle->GetStateVars(UIElementState::Idle);
			idleVars.BackgroundColor.SetValue(UIColor(0.2f, 0.2f, 0.2f, 1.f));
			idleVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			idleVars.Padding.SetValue(glm::vec4(10.f, 5.f, 10.f, 5.f));
			idleVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);

			auto& hoverVars = ButtonStyle->GetStateVars(UIElementState::Hovered);
			hoverVars.BackgroundColor.SetValue(UIColor(0.3f, 0.3f, 0.3f, 1.f));
			hoverVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			hoverVars.Padding.SetValue(glm::vec4(10.f, 5.f, 10.f, 5.f));
			hoverVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);

			auto& pressedVars = ButtonStyle->GetStateVars(UIElementState::Pressed);
			pressedVars.BackgroundColor.SetValue(UIColor(0.4f, 0.4f, 0.4f, 1.f));
			pressedVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			pressedVars.Padding.SetValue(glm::vec4(10.f, 5.f, 10.f, 5.f));
			pressedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			
			auto& disabledVars = ButtonStyle->GetStateVars(UIElementState::Disabled);
			disabledVars.BackgroundColor.SetValue(UIColor(0.1f, 0.1f, 0.1f, 1.f));
			disabledVars.ForegroundColor.SetValue(UIColor(0.5f, 0.5f, 0.5f, 1.f));
			disabledVars.Padding.SetValue(glm::vec4(10.f, 5.f, 10.f, 5.f));
			disabledVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);

			auto& selectedVars = ButtonStyle->GetStateVars(UIElementState::Selected);
			selectedVars.BackgroundColor.SetValue(UIColor(0.3f, 0.3f, 0.3f, 1.f));
			selectedVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			selectedVars.Padding.SetValue(glm::vec4(10.f, 5.f, 10.f, 5.f));
			selectedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);

			auto& focusedVars = ButtonStyle->GetStateVars(UIElementState::Focused);
			focusedVars.BackgroundColor.SetValue(UIColor(0.3f, 0.3f, 0.3f, 1.f));
			focusedVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			focusedVars.Padding.SetValue(glm::vec4(10.f, 5.f, 10.f, 5.f));
			focusedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
		}

		// Text
		{
			TextStyle = MakeRef<UITextStyle>();

			auto& idleVars = TextStyle->GetStateVars(UIElementState::Idle);
			idleVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			idleVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			idleVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			idleVars.FontSize.SetValue(16.f);

			auto& hoverVars = TextStyle->GetStateVars(UIElementState::Hovered);
			hoverVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			hoverVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			hoverVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			hoverVars.FontSize.SetValue(16.f);

			auto& pressedVars = TextStyle->GetStateVars(UIElementState::Pressed);
			pressedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			pressedVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			pressedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			pressedVars.FontSize.SetValue(16.f);

			auto& disabledVars = TextStyle->GetStateVars(UIElementState::Disabled);
			disabledVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			disabledVars.ForegroundColor.SetValue(UIColor(0.5f, 0.5f, 0.5f, 1.f));
			disabledVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			disabledVars.FontSize.SetValue(16.f);

			auto& selectedVars = TextStyle->GetStateVars(UIElementState::Selected);
			selectedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			selectedVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			selectedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			selectedVars.FontSize.SetValue(16.f);

			auto& focusedVars = TextStyle->GetStateVars(UIElementState::Focused);
			focusedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			focusedVars.ForegroundColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			focusedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			focusedVars.FontSize.SetValue(16.f);
		}
	}

	UITheme::~UITheme()
	{
	}

	Ref<UITheme> UITheme::GetDefault()
	{
		static Ref<UITheme> theme = nullptr;
		if (!theme)
		{
			theme = MakeRef<UITheme>();
		}

		return theme;
	}
}