#pragma once

namespace Mule
{
	template<typename ...Args>
	inline Ref<ScriptInstance> ScriptType::Instaniate(Args && ...args)
	{
		Coral::ManagedObject object = mType->CreateInstance(std::forward<Args>(args)...);

		return MakeRef<ScriptInstance>(mType->GetFullName(), object);
	}
}
