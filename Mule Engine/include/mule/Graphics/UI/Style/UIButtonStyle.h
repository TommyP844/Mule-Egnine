#pragma once

#include "WeakRef.h"

#include <optional>
#include <glm/glm.hpp>

#include "Graphics/UI/Style/UIColor.h"
#include "Graphics/UI/Style/UIBaseStyle.h"
#include "Graphics/UI/Style/StyleVariable.h"

namespace Mule
{
	struct UIButtonStyleVars
	{
		StyleVariable<UIColor> BackgroundColor;
		StyleVariable<UIColor> ForegroundColor;
		StyleVariable<glm::vec4> Padding;
		StyleVariable<AssetHandle> FontHandle;
	};

	class UIButtonStyle : public UIStyle<UIButtonStyleVars, UIElementType::UIButton>
	{
	public:
		UIButtonStyle() {}
		virtual ~UIButtonStyle() {}
	
		GET_STYLE_VAR_WITH_THEME(BackgroundColor, UIColor, UIButtonStyle);
		GET_STYLE_VAR_WITH_THEME(ForegroundColor, UIColor, UIButtonStyle);
		GET_STYLE_VAR_WITH_THEME(Padding, glm::vec4, UIButtonStyle);
		GET_STYLE_VAR_WITH_THEME(FontHandle, AssetHandle, UIButtonStyle);
	};
}