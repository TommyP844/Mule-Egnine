#include "Scripting/ScriptType.h"

#include <Coral/Attribute.hpp>

#include <spdlog/spdlog.h>

namespace Mule
{
	ScriptType::ScriptType(Coral::Type* type, Coral::ManagedAssembly* assembly)
		:
		mAssembly(assembly),
		mType(type)
	{
		for (auto field : type->GetFields())
		{
			ScriptField scriptField;

			scriptField.Name = field.GetName();
			scriptField.Type = GetFieldTypeFromString(field.GetType().GetFullName());
			scriptField.Accessibility = (FieldAccessibility)field.GetAccessibility();
			
			mFields[field.GetName()] = scriptField;
		}		
	}

	ScriptType::~ScriptType()
	{
	}
	
	bool ScriptType::DoesFieldExist(const std::string& fieldName) const
	{
		auto iter = mFields.find(fieldName);
		return iter != mFields.end();
	}
	
	const ScriptField& ScriptType::GetField(const std::string& fieldName) const
	{
		auto iter = mFields.find(fieldName);
		if (iter == mFields.end())
		{
			SPDLOG_ERROR("Field does not exists: {}", fieldName);
			return {};
		}

		return iter->second;
	}
	
	const std::unordered_map<std::string, ScriptField>& ScriptType::GetFields() const
	{
		return mFields;
	}
}