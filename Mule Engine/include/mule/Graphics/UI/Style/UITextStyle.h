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
		StyleVariable<UIColor> ForegroundColor;
		StyleVariable<UIColor> BackgroundColor;
		StyleVariable<float> FontSize;
		StyleVariable<AssetHandle> FontHandle;
	};

	class UITextStyle : public UIStyle<UITextStyleVars, UIElementType::UIText>
	{
	public:
		UITextStyle(){}
		virtual ~UITextStyle(){}

		GET_STYLE_VAR_WITH_THEME(BackgroundColor, UIColor, UITextStyle);
		GET_STYLE_VAR_WITH_THEME(ForegroundColor, UIColor, UITextStyle);
		GET_STYLE_VAR_WITH_THEME(FontSize, float, UITextStyle);
		GET_STYLE_VAR_WITH_THEME(FontHandle, AssetHandle, UITextStyle);
	};
}