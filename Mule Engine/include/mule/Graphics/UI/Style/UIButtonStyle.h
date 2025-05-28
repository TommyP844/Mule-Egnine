#pragma once

#include "WeakRef.h"

#include <optional>
#include <glm/glm.hpp>

#include "Graphics/UI/Style/UITextStyle.h"
#include "Graphics/UI/Style/UIColor.h"
#include "Graphics/UI/Style/UIBaseStyle.h"
#include "Graphics/UI/Style/StyleVariable.h"

namespace Mule
{
	struct UIButtonStyleVars
	{
		StyleVariable<UIColor> BackgroundColor;
		StyleVariable<UIPadding> Padding;
		StyleVariable<UIBorder> Border;
	};

	class UIButtonStyle : public UIStyle<UIButtonStyleVars, UIElementType::UIButton>
	{
	public:
		UIButtonStyle() {}
		virtual ~UIButtonStyle() {}

		GET_STYLE_VAR_WITH_THEME(BackgroundColor, UIColor, UIButtonStyle);
		GET_STYLE_VAR_WITH_THEME(Padding, UIPadding, UIButtonStyle);
		GET_STYLE_VAR_WITH_THEME(Border, UIBorder, UIButtonStyle);

		StyleVariable<Ref<UITextStyle>> TextStyle; // Reference to a text style for the button

	};
}