#include "Scripting/ScriptContext.h"

#include <spdlog/spdlog.h>
#include "Scripting/ScriptGlue.h"

#include <random>
#include <any>

#define ADD_INTERNAL_CALL(func) mEngineAssembly.AddInternalCall("Mule.InternalCalls", #func, &func);

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

		mEngineAssembly = mLoadContext.LoadAssembly("../MuleScriptEngine/bin/Debug/Mule.dll");

		if (mEngineAssembly.GetLoadStatus() != Coral::AssemblyLoadStatus::Success)
		{
			SPDLOG_ERROR("Failed to Load Mule Engine DLL");
			return;
		}

		SetContext(mEngineContext);
		UploadInternalCalls();

		for (auto type : mEngineAssembly.GetTypes())
		{
			std::string name = type->GetFullName();

			if (name == "Mule.InternalCalls")
				continue;
			if (name.find("`1")) // Skip Generics
				continue;

			mTypes[name] = type;
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

		mTypes.clear();
		for (auto type : mUserAssembly.GetTypes())
		{
			std::string name = type->GetFullName();
			mTypes[name] = type;
			for(auto method : type->GetMethods())
			mTypeMethods[name].insert(method.GetName());
		}
	}

	bool ScriptContext::DoesInstanceExist(Guid guid) const
	{
		auto iter = mScriptInstances.find(guid);
		return iter != mScriptInstances.end();
	}

	bool ScriptContext::CreateInstance(const std::string& name, Guid guid, const std::unordered_map<std::string, ScriptFieldInfo>& fieldData)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mTypes.find(name);
		if (iter == mTypes.end())
		{
			SPDLOG_ERROR("Failed to create script instance: {}", name);
			return false;
		}

		Coral::Type* type = iter->second;

		Coral::ManagedObject object = type->CreateInstance();
		uint64_t h = guid;
		object.SetFieldValue("_guid", h);

		if (mScriptInstances.find(guid) != mScriptInstances.end())
		{
			mScriptInstances.at(guid).Destroy();
		}

		for (auto& [name, field] : fieldData)
		{
			switch (field.GetType())
			{
			case ScriptFieldType::Int: object.SetFieldValue(name, field.GetValue<int32_t>()); break;
			case ScriptFieldType::Float: object.SetFieldValue(name, field.GetValue<float>()); break;
			case ScriptFieldType::Double: object.SetFieldValue(name, field.GetValue<double>()); break;
			case ScriptFieldType::String: object.SetFieldValue(name, field.GetValue<std::string>()); break;
			case ScriptFieldType::Vector2: object.SetFieldValue(name, field.GetValue<glm::vec2>()); break;
			case ScriptFieldType::Vector3: object.SetFieldValue(name, field.GetValue<glm::vec3>()); break;
			case ScriptFieldType::Vector4: object.SetFieldValue(name, field.GetValue<glm::vec4>()); break;
			}
		}

		mScriptInstances[guid] = object;

		return true;
	}

	bool ScriptContext::DoesTypeExist(const std::string& className) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mTypes.find(className);
		return iter != mTypes.end();
	}

	std::unordered_map<std::string, ScriptFieldInfo> ScriptContext::GetScriptFields(const std::string& name) const
	{
		std::unordered_map<std::string, ScriptFieldInfo> fields;

		auto iter = mTypes.find(name);
		if (iter == mTypes.end())
		{
			SPDLOG_WARN("Script Class does not exist: {}", name);
			return {};
		}

		Coral::Type* type = iter->second;
		for (auto field : type->GetFields())
		{
			ScriptFieldType type = GetTypeFromName(field.GetType().GetFullName());
			if (type == ScriptFieldType::None)
				continue;
			
			fields[field.GetName()] = ScriptFieldInfo(type);
		}

		return fields;
	}

	std::unordered_map<std::string, ScriptFieldInfo> ScriptContext::RefreshScriptFields(const std::string className, const std::unordered_map<std::string, ScriptFieldInfo>& fieldData) const
	{
		std::unordered_map<std::string, ScriptFieldInfo> fields = GetScriptFields(className);

		for (auto& [fieldName, field] : fields)
		{
			auto iter = fieldData.find(fieldName);
			if (iter == fieldData.end())
				continue;

			ScriptFieldInfo oldField = fieldData.at(fieldName);

			if (field.GetType() == oldField.GetType())
			{
				switch (field.GetType())
				{
				case ScriptFieldType::Int: field.SetValue(oldField.GetValue<int32_t>()); break;
				case ScriptFieldType::Float: field.SetValue(oldField.GetValue<float>()); break;
				case ScriptFieldType::Double: field.SetValue(oldField.GetValue<double>()); break;
				case ScriptFieldType::String: field.SetValue(oldField.GetValue<std::string>()); break;
				case ScriptFieldType::Vector2: field.SetValue(oldField.GetValue<glm::vec2>()); break;
				case ScriptFieldType::Vector3: field.SetValue(oldField.GetValue<glm::vec3>()); break;
				case ScriptFieldType::Vector4: field.SetValue(oldField.GetValue<glm::vec4>()); break;
				}
			}
		}

		return fields;
	}

	void ScriptContext::OnAwake(Guid guid) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		if (!DoesInstanceExist(guid))
		{
			SPDLOG_ERROR("Script instance does not exist");
			return;
		}

		bool hasOnAwake = ScriptHasMethod(guid, "Awake");
				
		if (hasOnAwake)
		{
			const Coral::ManagedObject& object = mScriptInstances.at(guid);
			object.InvokeMethod("Awake");
		}
		else
		{
			SPDLOG_WARN("Script instance does not have Awake method");
		}
	}

	void ScriptContext::OnStart(Guid guid) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		if (!DoesInstanceExist(guid))
		{
			SPDLOG_ERROR("Script instance does not exist");
			return;
		}
		bool hasOnStart = ScriptHasMethod(guid, "OnStart");

		if (hasOnStart)
		{
			const Coral::ManagedObject& object = mScriptInstances.at(guid);
			object.InvokeMethod("OnStart");
		}
	}

	void ScriptContext::OnUpdate(Guid guid, float dt) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		if (!DoesInstanceExist(guid))
		{
			SPDLOG_ERROR("Script instance does not exist");
			return;
		}
		bool hasOnUpdate = ScriptHasMethod(guid, "OnUpdate");

		if (hasOnUpdate)
		{
			const Coral::ManagedObject& object = mScriptInstances.at(guid);
			object.InvokeMethod("OnUpdate", dt);
		}
	}

	void ScriptContext::OnTriggerEnter(Guid guid, Guid other) const
	{
	}

	void ScriptContext::OnTriggerLeave(Guid guid, Guid other) const
	{
	}

	void ScriptContext::UploadInternalCalls()
	{	
		// Entity
		ADD_INTERNAL_CALL(GetComponentPtr);
		ADD_INTERNAL_CALL(AddComponentGetPtr);
		ADD_INTERNAL_CALL(HasComponent);
		ADD_INTERNAL_CALL(RemoveComponent);
		
		// Input
		ADD_INTERNAL_CALL(SetMousePos);
		ADD_INTERNAL_CALL(GetMousePosX);
		ADD_INTERNAL_CALL(GetMousePosY);
		ADD_INTERNAL_CALL(IsMouseButtonPressed);
		ADD_INTERNAL_CALL(IsKeyDown);

		// Camera
		ADD_INTERNAL_CALL(UpdateCameraProjectionMatrix);
		ADD_INTERNAL_CALL(UpdateCameraVectors);
		ADD_INTERNAL_CALL(UpdateCameraViewMatrix);
		ADD_INTERNAL_CALL(UpdateCameraVPMatrix);

		// Physics
		ADD_INTERNAL_CALL(SetRigidBodyMass);
		ADD_INTERNAL_CALL(GetRigidBodyMass);
		ADD_INTERNAL_CALL(MoveRigidBodyKinematic);
		ADD_INTERNAL_CALL(AddRigidBodyForce);
		ADD_INTERNAL_CALL(AddRigidBodyImpulse);
		ADD_INTERNAL_CALL(AddRigidBodyTorque);
		ADD_INTERNAL_CALL(AddRigidBodyAngularImpulse);
		ADD_INTERNAL_CALL(SetRigidBodyLinearVelocity);
		ADD_INTERNAL_CALL(SetRigidBodyAngularVelocity);
		ADD_INTERNAL_CALL(GetRigidBodyLinearVelocity);

		mEngineAssembly.UploadInternalCalls();
	}

	bool ScriptContext::ScriptHasMethod(Guid guid, const std::string& methodName) const
	{
		Coral::ManagedObject object = mScriptInstances.at(guid);
		const Coral::Type& type = object.GetType();
		const std::set<std::string>& methods = mTypeMethods.at(type.GetFullName());
		return methods.contains(methodName);
	}

	ScriptFieldType ScriptContext::GetTypeFromName(const std::string& name) const
	{
		if(name == "System.Int32")
			return ScriptFieldType::Int;
		else if(name == "System.Single")
			return ScriptFieldType::Float;
		else if (name == "System.Double")
			return ScriptFieldType::Double;
		else if (name == "System.String")
			return ScriptFieldType::String;
		else if (name == "System.Numerics.Vector2")
			return ScriptFieldType::Vector2;
		else if (name == "System.Numerics.Vector3")
			return ScriptFieldType::Vector3;
		else if (name == "System.Numerics.Vector4")
			return ScriptFieldType::Vector4;
		else if (name == "Mule..Entity")
			return ScriptFieldType::Entity;

		return ScriptFieldType::None;
	}

}