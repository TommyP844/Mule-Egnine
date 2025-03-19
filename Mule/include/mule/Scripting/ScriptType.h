#pragma once

#include "Ref.h"
#include "ScriptInstance.h"
#include "ScriptField.h"

#include <Coral/Type.hpp>
#include <Coral/Assembly.hpp>

namespace Mule
{
	class ScriptType
	{
	public:
		ScriptType() = default;
		ScriptType(Coral::Type* type, Coral::ManagedAssembly* assembly);
		~ScriptType();

		template<typename... Args>
		Ref<ScriptInstance> Instaniate(Args&&... args);

		bool DoesFieldExist(const std::string& fieldName) const;
		const ScriptField& GetField(const std::string& fieldName) const;
		const std::unordered_map<std::string, ScriptField>& GetFields() const;

	private:
		friend class ScriptContext;

		Coral::Type* mType;
		Coral::ManagedAssembly* mAssembly;

		std::unordered_map<std::string, ScriptField> mFields;
	};
}

#include "ScriptType.inl"
