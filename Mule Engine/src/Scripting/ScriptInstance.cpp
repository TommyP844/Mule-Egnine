#include "Scripting/ScriptInstance.h"

namespace Mule
{
	ScriptInstance::ScriptInstance(const std::string& className, Coral::ManagedObject object)
		:
		mObject(object),
		mClassName(className)
	{
	}

	ScriptInstance::~ScriptInstance()
	{
		mObject.Destroy();
	}

	void ScriptInstance::OnStart()
	{
		mObject.InvokeMethod("OnStart");
	}

	void ScriptInstance::OnUpdate(float dt)
	{
		mObject.InvokeMethod("OnUpdate", dt);
	}
}