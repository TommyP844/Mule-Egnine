#pragma once

#include "UIUnitType.h"

namespace Mule
{
	struct UIMeasurement
	{
		UIMeasurement() = default;
		UIMeasurement(float value, UIUnitType unitType)
			:
			Value(value),
			mUnitType(unitType)
		{ }

		float Value = 5.f;

		float Resolve(float parentSize) const
		{
			if (mUnitType == UIUnitType::Percentage)
				return (Value / 100.f) * parentSize;

			return Value;
		}

		UIUnitType GetUnitType() const { return mUnitType; }
		
		void SetUnitType(UIUnitType type, float parentSize)
		{
			if (type == mUnitType)
				return;

			switch (type)
			{
			case UIUnitType::Pixels:
				Value = (Value / 100.f) * parentSize;
				break;
			case UIUnitType::Percentage:
				Value = (Value / parentSize) * 100.f;
				break;
			}

			mUnitType = type;
		}

		void Init(float value, UIUnitType type)
		{
			Value = value;
			mUnitType = type;
		}

	private:
		UIUnitType mUnitType = UIUnitType::Pixels;
	};
}