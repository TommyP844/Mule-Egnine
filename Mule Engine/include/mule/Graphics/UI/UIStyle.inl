
#include <assert.h>

#include "UITheme.h"

namespace Mule
{
	template<typename T>
	inline void UIStyle::SetValue(UIStyleKey key, const T& value)
	{
		size_t index = static_cast<size_t>(key);
		mValid[index] = true;
		mValues[index] = value;
	}

	template<typename T>
	inline T UIStyle::GetValue(UIStyleKey key, WeakRef<UITheme> theme) const
	{
		size_t index = static_cast<size_t>(key);
		if (mValid[index]) return std::get<T>(mValues[index]);
		return theme->GetValue<T>(key);
	}
}