#include "Scripting/ScriptContext.h"

#include <spdlog/spdlog.h>
#include "Scripting/ScriptGlue.h"

#include <random>
#include <any>

namespace Mule
{
	void ExceptionCallback(std::string_view InMessage)
	{
		SPDLOG_ERROR("C# Exception: {}", InMessage);
	}

	ScriptContext::ScriptContext(WeakRef<EngineContext> context)
		:
		mInitSuccess(false),
		mEngineContext(context)
	{
		Coral::HostSettings settings = {
			.CoralDirectory = "../Submodules/Coral/Build/Debug",
			.ExceptionCallback = ExceptionCallback
		};

		mInstance.Initialize(settings);

		mLoadContext = mInstance.CreateAssemblyLoadContext("Mule Context");
		mUserLoadContext = mInstance.CreateAssemblyLoadContext("User Context");

		mEngineAssembly = mLoadContext.LoadAssembly("../MuleScriptEngine/bin/Debug/MuleScriptEngine.dll");

		if (mEngineAssembly.GetLoadStatus() != Coral::AssemblyLoadStatus::Success)
		{
			SPDLOG_ERROR("Failed to Load Mule Engine DLL");
			return;
		}

		Scripting::SetContext(mEngineContext);
		UploadInternalCalls();

		for (auto type : mEngineAssembly.GetTypes())
		{
			std::string name = type->GetFullName();

			if (name == "Mule.InternalCalls")
				continue;

			mTypes[name] = ScriptType(type, &mEngineAssembly);
		}
	}

	ScriptContext::~ScriptContext()
	{
	}

	void ScriptContext::LoadUserDLL(const fs::path& dllPath)
	{
		mUserAssemblyPath = dllPath;
		ReloadDLL();
	}

	void ScriptContext::ReloadDLL()
	{
		// std::lock_guard<std::mutex> lock(mMutex);

		SPDLOG_INFO("Reloading dll");

		if (!fs::exists(mUserAssemblyPath))
		{
			SPDLOG_WARN("DLL Path does not exist: {}", mUserAssemblyPath.string());
			return;
		}

		struct ClassMemeber
		{
			std::any Value;
			ScriptFieldType Type;
		};

		struct ClassInstance
		{
			std::string ClassName;
			uint32_t Id;
			std::unordered_map<std::string, ClassMemeber> Fields;
		};

		std::unordered_map<ScriptHandle, ClassInstance> scriptValues;
		for (auto [handle, instance] : mScriptInstances)
		{
			auto& oldInstance = scriptValues[handle];
			oldInstance.ClassName = instance.second->GetName();
			oldInstance.Id = instance.first;
			auto& valueMap = oldInstance.Fields;
			auto type = mTypes[instance.second->mClassName];

#define GET_FIELD_AS_ANY(enumType, type) { std::make_any<type>(instance.second->GetFieldValue<type>(fieldName)), enumType };

			for (auto [fieldName, field] : type.GetFields())
			{
				if (field.Accessibility != FieldAccessibility::Public)
					continue;

				switch (field.Type)
				{
				case ScriptFieldType::Bool: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::Bool, bool); break;
				case ScriptFieldType::Byte: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::Byte, unsigned char); break;
				case ScriptFieldType::Int16: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::Int16, int16_t); break;
				case ScriptFieldType::UInt16: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::UInt16, uint16_t); break;
				case ScriptFieldType::Int32: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::Int32, int32_t); break;
				case ScriptFieldType::UInt32: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::UInt32, uint32_t); break;
				case ScriptFieldType::Int64: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::Int64, int64_t); break;
				case ScriptFieldType::UInt64: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::UInt64, uint64_t); break;
				case ScriptFieldType::Float: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::Float, float); break;
				case ScriptFieldType::Double: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::Double, double); break;
				case ScriptFieldType::String: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::String, std::string); break;
				case ScriptFieldType::IntPtr: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::IntPtr, void*); break;
				case ScriptFieldType::UIntPtr: valueMap[fieldName] = GET_FIELD_AS_ANY(ScriptFieldType::UIntPtr, void*); break;
				}
				
			}

			instance.second->mObject.Destroy();
		}

		if (mUserAssembly.GetLoadStatus() == Coral::AssemblyLoadStatus::Success)
		{
			mInstance.UnloadAssemblyLoadContext(mUserLoadContext);
			mUserLoadContext = mInstance.CreateAssemblyLoadContext("User Context");
		}

		mUserAssembly = mUserLoadContext.LoadAssembly(mUserAssemblyPath.string());

		if (mUserAssembly.GetLoadStatus() != Coral::AssemblyLoadStatus::Success)
		{
			SPDLOG_ERROR("Failed to Load Mule Engine DLL");
			return;
		}

		for (auto type : mUserAssembly.GetTypes())
		{
			std::string name = type->GetFullName();
			mTypes[name] = ScriptType(type, &mUserAssembly);
		}

		for (auto [handle, instanceData] : scriptValues)
		{
			// User may have deleted class
			auto iter = mTypes.find(instanceData.ClassName);
			if (iter == mTypes.end())
				continue;

			auto& type = iter->second;
			
			bool success = CreateInstance(instanceData.ClassName, instanceData.Id, handle);
			if (!success)
			{
				SPDLOG_ERROR("Failed to copy script, {}", instanceData.ClassName);
				continue;
			}

			auto instance = GetScriptInstance(handle);


			for (auto [name, member] : instanceData.Fields)
			{
				if (!type.DoesFieldExist(name))
					continue;

				const auto& field = type.GetField(name);
				if (field.Accessibility != FieldAccessibility::Public)
					continue;

				if (field.Type != member.Type)
					continue;

				

				switch (member.Type)
				{
				case ScriptFieldType::Bool:		instance->SetFieldValue<bool>(name, std::any_cast<bool>(member.Value));	break;
				case ScriptFieldType::Byte:		instance->SetFieldValue<unsigned char>(name, std::any_cast<unsigned char>(member.Value)); break;
				case ScriptFieldType::Int16:	instance->SetFieldValue<int16_t>(name, std::any_cast<int16_t>(member.Value));	break;
				case ScriptFieldType::UInt16:	instance->SetFieldValue<uint16_t>(name, std::any_cast<uint16_t>(member.Value));	break;
				case ScriptFieldType::Int32:	instance->SetFieldValue<int32_t>(name, std::any_cast<int32_t>(member.Value));	break;
				case ScriptFieldType::UInt32:	instance->SetFieldValue<uint32_t>(name, std::any_cast<uint32_t>(member.Value));	break;
				case ScriptFieldType::Int64:	instance->SetFieldValue<int64_t>(name, std::any_cast<int64_t>(member.Value));	break;
				case ScriptFieldType::UInt64:	instance->SetFieldValue<uint64_t>(name, std::any_cast<uint64_t>(member.Value));	break;
				case ScriptFieldType::Float:	instance->SetFieldValue<float>(name, std::any_cast<float>(member.Value));	break;
				case ScriptFieldType::Double:	instance->SetFieldValue<double>(name, std::any_cast<double>(member.Value));	break;
				case ScriptFieldType::String:	instance->SetFieldValue<std::string>(name, std::any_cast<std::string>(member.Value));	break;
				case ScriptFieldType::IntPtr:	instance->SetFieldValue<void*>(name, std::any_cast<void*>(member.Value));	break;
				case ScriptFieldType::UIntPtr:	instance->SetFieldValue<void*>(name, std::any_cast<void*>(member.Value));	break;
				}
			}
		}
	}

	ScriptHandle ScriptContext::CreateInstance(const std::string& name, uint32_t id)
	{
		ScriptHandle handle = GenerateScriptHandle();
		if (!CreateInstance(name, id, handle))
		{
			return NullScriptHandle;
		}

		return handle;
	}

	bool ScriptContext::CreateInstance(const std::string& name, uint32_t id, ScriptHandle handle)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mTypes.find(name);
		if (iter == mTypes.end())
		{
			SPDLOG_ERROR("Failed to create script instance: {}", name);
			return false;
		}

		Coral::Type* type = iter->second.mType;

		Coral::ManagedObject object = type->CreateInstance();
		object.SetFieldValue<uint32_t>("_id", id);

		Ref<ScriptInstance> instance = MakeRef<ScriptInstance>(type->GetFullName(), object);

		mScriptInstances[handle] = { id, instance };

		return true;
	}

	WeakRef<ScriptInstance> ScriptContext::GetScriptInstance(ScriptHandle handle)
	{
		if (handle == NullScriptHandle)
			return nullptr;

		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mScriptInstances.find(handle);
		if (iter == mScriptInstances.end())
			return nullptr;

		return iter->second.second;
	}

	const ScriptType& ScriptContext::GetType(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mTypes.find(name);
		if (iter == mTypes.end())
		{
			return {};
		}
		return iter->second;
	}

	bool ScriptContext::DoesTypeExist(const std::string& className) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mTypes.find(className);
		return iter != mTypes.end();
	}

	ScriptHandle ScriptContext::GenerateScriptHandle()
	{
		std::random_device device;
		std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);
		ScriptHandle handle = dist(device);

		return handle;
	}

	void ScriptContext::UploadInternalCalls()
	{	
		mEngineAssembly.AddInternalCall("Mule.InternalCalls", "GetComponentPtr", &Scripting::GetComponentPtr);
		mEngineAssembly.AddInternalCall("Mule.InternalCalls", "AddComponentGetPtr", &Scripting::AddComponentGetPtr);
		mEngineAssembly.AddInternalCall("Mule.InternalCalls", "HasComponent", &Scripting::HasComponent);
		mEngineAssembly.AddInternalCall("Mule.InternalCalls", "RemoveComponent", &Scripting::RemoveComponent);

		mEngineAssembly.UploadInternalCalls();
	}

}