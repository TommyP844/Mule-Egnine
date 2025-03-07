
#include <string>
#include <filesystem>

#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>

namespace Mule
{
	class ScriptContext
	{
	public:
		ScriptContext();
		~ScriptContext();

	private:
		Coral::HostInstance mInstance;
		Coral::AssemblyLoadContext mLoadContext;

		Coral::ManagedAssembly mEngineAssembly;
		Coral::ManagedAssembly mUserAssembly;

	};
}