#include "Scripting/ScriptContext.h"

#include <spdlog/spdlog.h>

#include <string>

namespace Mule
{
	void ExceptionCallback(std::string_view InMessage)
	{
		SPDLOG_ERROR("C# Exception: {}", InMessage);
	}

	ScriptContext::ScriptContext()
	{
		Coral::HostSettings settings = {
			.CoralDirectory = "../Submodules/Coral/Coral.Managed",
			.ExceptionCallback = ExceptionCallback
		};

		mInstance.Initialize(settings);

		mLoadContext = mInstance.CreateAssemblyLoadContext("Mule Context");

		mEngineAssembly = mLoadContext.LoadAssembly("");

	}

	ScriptContext::~ScriptContext()
	{
	}
}