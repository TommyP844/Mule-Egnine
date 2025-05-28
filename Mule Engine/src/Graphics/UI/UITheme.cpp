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
			idleVars.Padding.SetValue({
				.Left = UIMeasurement(10.f, UIUnitType::Pixels),
				.Right = UIMeasurement(10.f, UIUnitType::Pixels),
				.Top = UIMeasurement(5.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(5.f, UIUnitType::Pixels)
				});
			idleVars.Border.SetValue({
				.Thickness = UIMeasurement(1.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.5f, 0.5f, 0.5f, 1.f)
				});

			auto& hoverVars = ButtonStyle->GetStateVars(UIElementState::Hovered);
			hoverVars.BackgroundColor.SetValue(UIColor(0.3f, 0.3f, 0.3f, 1.f));
			hoverVars.Padding.SetValue({
				.Left = UIMeasurement(10.f, UIUnitType::Pixels),
				.Right = UIMeasurement(10.f, UIUnitType::Pixels),
				.Top = UIMeasurement(5.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(5.f, UIUnitType::Pixels)
				});
			hoverVars.Border.SetValue({
				.Thickness = UIMeasurement(1.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.5f, 0.5f, 0.5f, 1.f)
				});

			auto& pressedVars = ButtonStyle->GetStateVars(UIElementState::Pressed);
			pressedVars.BackgroundColor.SetValue(UIColor(0.4f, 0.4f, 0.4f, 1.f));
			pressedVars.Padding.SetValue({
				.Left = UIMeasurement(10.f, UIUnitType::Pixels),
				.Right = UIMeasurement(10.f, UIUnitType::Pixels),
				.Top = UIMeasurement(5.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(5.f, UIUnitType::Pixels)
				});
			pressedVars.Border.SetValue({
				.Thickness = UIMeasurement(1.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.5f, 0.5f, 0.5f, 1.f)
				});
			
			auto& disabledVars = ButtonStyle->GetStateVars(UIElementState::Disabled);
			disabledVars.BackgroundColor.SetValue(UIColor(0.1f, 0.1f, 0.1f, 1.f));
			disabledVars.Padding.SetValue({
				.Left = UIMeasurement(10.f, UIUnitType::Pixels),
				.Right = UIMeasurement(10.f, UIUnitType::Pixels),
				.Top = UIMeasurement(5.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(5.f, UIUnitType::Pixels)
				});
			disabledVars.Border.SetValue({
				.Thickness = UIMeasurement(1.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.5f, 0.5f, 0.5f, 1.f)
				});

			auto& selectedVars = ButtonStyle->GetStateVars(UIElementState::Selected);
			selectedVars.BackgroundColor.SetValue(UIColor(0.3f, 0.3f, 0.3f, 1.f));
			selectedVars.Padding.SetValue({
				.Left = UIMeasurement(10.f, UIUnitType::Pixels),
				.Right = UIMeasurement(10.f, UIUnitType::Pixels),
				.Top = UIMeasurement(5.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(5.f, UIUnitType::Pixels)
				});
			selectedVars.Border.SetValue({
				.Thickness = UIMeasurement(1.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.5f, 0.5f, 0.5f, 1.f)
				});

			auto& focusedVars = ButtonStyle->GetStateVars(UIElementState::Focused);
			focusedVars.BackgroundColor.SetValue(UIColor(0.3f, 0.3f, 0.3f, 1.f));
			focusedVars.Padding.SetValue({
				.Left = UIMeasurement(10.f, UIUnitType::Pixels),
				.Right = UIMeasurement(10.f, UIUnitType::Pixels),
				.Top = UIMeasurement(5.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(5.f, UIUnitType::Pixels)
				});
			focusedVars.Border.SetValue({
				.Thickness = UIMeasurement(1.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.5f, 0.5f, 0.5f, 1.f)
				});

			ButtonStyle->TextStyle.SetValue(MakeRef<UITextStyle>());
			auto& buttonTextStyle = ButtonStyle->TextStyle.GetValue();
			auto& buttonIdleVars = buttonTextStyle->GetStateVars(UIElementState::Idle);
			buttonIdleVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			buttonIdleVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			buttonIdleVars.FontSize.SetValue(16.f);
			buttonIdleVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			buttonIdleVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			buttonIdleVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& buttonHoverVars = buttonTextStyle->GetStateVars(UIElementState::Hovered);
			buttonHoverVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			buttonHoverVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			buttonHoverVars.FontSize.SetValue(16.f);
			buttonHoverVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			buttonHoverVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			buttonHoverVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& buttonPressedVars = buttonTextStyle->GetStateVars(UIElementState::Pressed);
			buttonPressedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			buttonPressedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			buttonPressedVars.FontSize.SetValue(16.f);
			buttonPressedVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			buttonPressedVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			buttonPressedVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& buttonDisabledVars = buttonTextStyle->GetStateVars(UIElementState::Disabled);
			buttonDisabledVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			buttonDisabledVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			buttonDisabledVars.FontSize.SetValue(16.f);
			buttonDisabledVars.FontColor.SetValue(UIColor(0.5f, 0.5f, 0.5f, 1.f));
			buttonDisabledVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			buttonDisabledVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& buttonSelectedVars = buttonTextStyle->GetStateVars(UIElementState::Selected);
			buttonSelectedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			buttonSelectedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			buttonSelectedVars.FontSize.SetValue(16.f);
			buttonSelectedVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			buttonSelectedVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			buttonSelectedVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& buttonFocusedVars = buttonTextStyle->GetStateVars(UIElementState::Focused);
			buttonFocusedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			buttonFocusedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			buttonFocusedVars.FontSize.SetValue(16.f);
			buttonFocusedVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			buttonFocusedVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			buttonFocusedVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

		}

		// Text
		{
			TextStyle = MakeRef<UITextStyle>();

			auto& idleVars = TextStyle->GetStateVars(UIElementState::Idle);
			idleVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			idleVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			idleVars.FontSize.SetValue(16.f);
			idleVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			idleVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			idleVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& hoverVars = TextStyle->GetStateVars(UIElementState::Hovered);
			hoverVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			hoverVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			hoverVars.FontSize.SetValue(16.f);
			hoverVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			hoverVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			hoverVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& pressedVars = TextStyle->GetStateVars(UIElementState::Pressed);
			pressedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			pressedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			pressedVars.FontSize.SetValue(16.f);
			pressedVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			pressedVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			pressedVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& disabledVars = TextStyle->GetStateVars(UIElementState::Disabled);
			disabledVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			disabledVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			disabledVars.FontSize.SetValue(16.f);
			disabledVars.FontColor.SetValue(UIColor(0.5f, 0.5f, 0.5f, 1.f));
			disabledVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			disabledVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& selectedVars = TextStyle->GetStateVars(UIElementState::Selected);
			selectedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			selectedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			selectedVars.FontSize.SetValue(16.f);
			selectedVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			selectedVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			selectedVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});

			auto& focusedVars = TextStyle->GetStateVars(UIElementState::Focused);
			focusedVars.BackgroundColor.SetValue(UIColor(0.f, 0.f, 0.f, 0.f));
			focusedVars.FontHandle.SetValue(MULE_DEFAULT_FONT_HANDLE);
			focusedVars.FontSize.SetValue(16.f);
			focusedVars.FontColor.SetValue(UIColor(1.f, 1.f, 1.f, 1.f));
			focusedVars.Padding.SetValue({
				.Left = UIMeasurement(5.f, UIUnitType::Pixels),
				.Right = UIMeasurement(5.f, UIUnitType::Pixels),
				.Top = UIMeasurement(2.f, UIUnitType::Pixels),
				.Bottom = UIMeasurement(2.f, UIUnitType::Pixels)
				});
			focusedVars.Border.SetValue({
				.Thickness = UIMeasurement(0.f, UIUnitType::Pixels),
				.Radius = UIMeasurement(0.f, UIUnitType::Pixels),
				.Color = UIColor(0.f, 0.f, 0.f, 0.f)
				});
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