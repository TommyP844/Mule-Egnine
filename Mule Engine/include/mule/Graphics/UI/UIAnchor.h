#pragma once

#include "UIHandle.h"
#include "UIAnchorAxis.h"
#include "WeakRef.h"

namespace Mule
{

	struct UIAnchor
	{
		UIHandle TargetElement;
		UIAnchorAxis Target;
		UIAnchorAxis Self;
	};
}