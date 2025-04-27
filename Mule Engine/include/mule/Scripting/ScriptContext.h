#pragma once

#include "Ref.h"
#include "WeakRef.h"

#include <string>
#include <filesystem>
#include <unordered_map>
#include <set>

#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>

#include "Engine Context/EngineContext.h"
#include "Scripting/ScriptFieldInfo.h"

#include "ECS/Guid.h"

namespace fs = std::filesystem;

namespace Mule
{
	class ScriptContext
	{
	public:
		ScriptContext(WeakRef<EngineContext> context);
		~ScriptContext();

		void LoadUserDLL(const fs::path& dll);
		void ReloadDLL();

		bool DoesInstanceExist(Guid guid) const;
		bool CreateInstance(const std::string& name, Guid guid, const std::unordered_map<std::string, ScriptFieldInfo>& fieldData = {});
		bool DoesTypeExist(const std::string& className) const;

		std::unordered_map<std::string, ScriptFieldInfo> GetScriptFields(const std::string& className) const;
		std::unordered_map<std::string, ScriptFieldInfo> RefreshScriptFields(const std::string className, const std::unordered_map<std::string, ScriptFieldInfo>& fieldData) const;

		void OnAwake(Guid guid) const;
		void OnStart(Guid guid) const;
		void OnUpdate(Guid guid, float dt) const;
		void OnTriggerEnter(Guid guid, Guid other) const;
		void OnTriggerLeave(Guid guid, Guid other) const;

	private:
		mutable std::mutex mMutex;

		bool mInitSuccess;
		WeakRef<EngineContext> mEngineContext;

		Coral::HostInstance mInstance;
		Coral::AssemblyLoadContext mLoadContext;
		Coral::AssemblyLoadContext mUserLoadContext;
		Coral::ManagedAssembly mEngineAssembly;
		Coral::ManagedAssembly mUserAssembly;
		fs::path mUserAssemblyPath;
		std::unordered_map<Guid, Coral::ManagedObject> mScriptInstances;

		std::unordered_map<std::string, Coral::Type*> mTypes;
		std::unordered_map<std::string, std::set<std::string>> mTypeMethods;

		void UploadInternalCalls();
		bool ScriptHasMethod(Guid guid, const std::string& methodName) const;

		ScriptFieldType GetTypeFromName(const std::string& name) const;
	};
}