#pragma once

#include "UIAnchorAxis.h"
#include "WeakRef.h"

namespace Mule
{
	class UIElement;

	struct UIAnchor
	{
		WeakRef<UIElement> TargetElement;
		UIAnchorAxis Target;
		UIAnchorAxis Self;
	};
}