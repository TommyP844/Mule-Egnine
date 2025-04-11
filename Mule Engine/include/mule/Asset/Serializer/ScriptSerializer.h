#pragma once

#include "Scripting/ScriptClass.h"
#include "IAssetSerializer.h"

namespace Mule
{
	class ScriptSerializer : public IAssetSerializer<ScriptClass, AssetType::Script>
	{
	public:
		ScriptSerializer(WeakRef<ServiceManager> serviceManager);
		virtual ~ScriptSerializer() {}

		Ref<ScriptClass> Load(const fs::path& filepath) override;
		void Save(Ref<ScriptClass> asset) override;
	};
}
