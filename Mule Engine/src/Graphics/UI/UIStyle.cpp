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

	void UIStyle::SetParentStyle(Ref<UIStyle> parentStyle)
	{
		assert(parentStyle.Get() != this && "Parent Style cannot be equal to itself");
		mParentStyle = parentStyle;
	}

	Ref<UIStyle> UIStyle::GetParentStyle() const
	{
		return mParentStyle;
	}

	bool UIStyle::HasValue(UIStyleKey key) const
	{
		return mValid[static_cast<uint64_t>(key)];
	}
}