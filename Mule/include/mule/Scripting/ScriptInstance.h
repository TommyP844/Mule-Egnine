#pragma once

#include <string>
#include <Coral/ManagedObject.hpp>

namespace Mule
{
	class ScriptInstance
	{
	public:
		ScriptInstance(const std::string& className, Coral::ManagedObject object);
		~ScriptInstance();


		void OnStart();
		void OnUpdate(float dt);

		template<typename T>
		void SetFieldValue(const std::string& name, T value);

		template<typename T>
		T GetFieldValue(const std::string& name);

		Coral::ManagedObject GetManagedObject() const { return mObject; }
		const std::string& GetName() const { return mClassName; }
	private:
		friend class ScriptContext;

		const std::string mClassName;
		Coral::ManagedObject mObject;
	};
}

#include "ScriptInstance.inl"