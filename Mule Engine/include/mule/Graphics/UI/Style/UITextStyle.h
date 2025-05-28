#pragma once

#include "WeakRef.h"
#include "UIColor.h"
#include "UIBaseStyle.h"
#include "StyleVariable.h"
#include <optional>

#include <glm/glm.hpp>

namespace Mule
{
	struct UITextStyleVars
	{
		StyleVariable<UIColor> BackgroundColor;
		StyleVariable<UIPadding> Padding;
		StyleVariable<UIBorder> Border;
		StyleVariable<UIColor> FontColor;
		StyleVariable<float> FontSize;
		StyleVariable<AssetHandle> FontHandle;
	};

	class UITextStyle : public UIStyle<UITextStyleVars, UIElementType::UIText>
	{
	public:
		UITextStyle(){}
		virtual ~UITextStyle(){}

		GET_STYLE_VAR_WITH_THEME(BackgroundColor, UIColor, UITextStyle)
		GET_STYLE_VAR_WITH_THEME(Padding, UIPadding, UITextStyle)
		GET_STYLE_VAR_WITH_THEME(Border, UIBorder, UITextStyle)
		GET_STYLE_VAR_WITH_THEME(FontColor, UIColor, UITextStyle)
		GET_STYLE_VAR_WITH_THEME(FontSize, float, UITextStyle)
		GET_STYLE_VAR_WITH_THEME(FontHandle, AssetHandle, UITextStyle)
	};
}