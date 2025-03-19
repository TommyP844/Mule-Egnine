#include "ScriptInstance.h"
#pragma once

namespace Mule
{
	template<typename T>
	inline void Mule::ScriptInstance::SetFieldValue(const std::string& name, T value)
	{
		mObject.SetFieldValue<T>(name, value);
	}

	template<typename T>
	inline T Mule::ScriptInstance::GetFieldValue(const std::string& name)
	{
		return mObject.GetFieldValue<T>(name);
	}
}