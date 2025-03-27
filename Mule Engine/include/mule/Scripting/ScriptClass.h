#pragma once

#include "Asset/Asset.h"

namespace Mule
{
	class ScriptClass : public Asset<AssetType::Script>
	{
	public:
		ScriptClass(const fs::path& filePath) : Asset(filePath) {}
		virtual ~ScriptClass(){}
	};
}
