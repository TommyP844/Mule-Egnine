#pragma once

#include "Ref.h"
#include "WeakRef.h"

#include "ScriptInstance.h"
#include "ScriptType.h"

#include <string>
#include <filesystem>
#include <unordered_map>
#include <set>

#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>

#include "Engine Context/EngineContext.h"

namespace fs = std::filesystem;

namespace Mule
{
	typedef uint64_t ScriptHandle;
	constexpr ScriptHandle NullScriptHandle = 0;

	class ScriptContext
	{
	public:
		ScriptContext(WeakRef<EngineContext> context);
		~ScriptContext();

		void LoadUserDLL(const fs::path& dll);
		void ReloadDLL();

		ScriptHandle CreateInstance(const std::string& name, Guid guid);
		bool CreateInstance(const std::string& name, Guid guid, ScriptHandle handle);

		WeakRef<ScriptInstance> GetScriptInstance(ScriptHandle handle);

		bool DoesTypeExist(const std::string& className) const;
		const ScriptType& GetType(const std::string& name);

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
		std::unordered_map<ScriptHandle, std::pair<Guid, Ref<ScriptInstance>>> mScriptInstances;
		std::unordered_map<std::string, ScriptType> mTypes;

		void UploadInternalCalls();
		ScriptHandle GenerateScriptHandle();
	};
}