#pragma once

#include <optional>

namespace Mule
{
	template<typename T>
	class StyleVariable
	{
	public:
		StyleVariable()
			:
			mValue(std::nullopt)
		{ }

		void SetValue(const T& value)
		{
			mValue = value;
		}

		bool HasValue() const
		{
			return mValue.has_value();
		}
		
		const T& GetValue() const
		{
			return *mValue;
		}

		T& GetValue()
		{
			return *mValue;
		}

	private:
		std::optional<T> mValue;
	};
}
