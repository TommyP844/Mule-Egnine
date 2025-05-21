#pragma once


namespace Mule
{
	template<typename T>
	inline void UITheme::SetValue(UIStyleKey key, const T& value)
	{
		size_t index = static_cast<size_t>(key);
		mValues[index] = value;
	}

	template<typename T>
	inline T UITheme::GetValue(UIStyleKey key) const
	{
		size_t index = static_cast<size_t>(key);
		return std::get<T>(mValues[index]);
	}
}