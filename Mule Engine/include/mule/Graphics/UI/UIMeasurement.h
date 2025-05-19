#pragma once

#include <type_traits> // uint32_t

namespace Mule
{
	enum class UIUnitType : uint32_t
	{
		Pixels,
		Percentage
	};

	struct UIMeasurement
	{
		UIMeasurement() = default;
		UIMeasurement(float value, UIUnitType unitType)
			:
			Value(value),
			UnitType(unitType)
		{ }

		float Value = 0.f;
		UIUnitType UnitType = UIUnitType::Percentage;

		float Resolve(float parentSize) const
		{
			if (UnitType == UIUnitType::Percentage)
				return (Value / 100.f) * parentSize;

			return Value;
		}
	};
}