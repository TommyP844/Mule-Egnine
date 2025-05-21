#include "Graphics/UI/UIStyle.h"

namespace Mule
{
	UIStyle::UIStyle(const std::string& name)
		:
		Asset(name)
	{
		for (uint32_t i = 0; i < mValid.size(); i++)
			mValid[i] = false;
	}

	bool UIStyle::HasValue(UIStyleKey key) const
	{
		return mValid[static_cast<uint64_t>(key)];
	}

	Ref<UIStyle> UIStyle::GetDefault()
	{
		static Ref<UIStyle> style = nullptr;
		if (!style)
			style = MakeRef<UIStyle>("Default");

		return style;
	}
}