#pragma once

#include "Scripting/ScriptClass.h"
#include "AssetLoader.h"

namespace Mule
{
	class ScriptLoader : public IAssetLoader<ScriptClass, AssetType::Script>
	{
	public:
		ScriptLoader() {}
		virtual ~ScriptLoader() {}

		Ref<ScriptClass> LoadText(const fs::path& filepath) override;
		void SaveText(Ref<ScriptClass> asset) override;
		Ref<ScriptClass> LoadBinary(const Buffer& buffer) override;
		void SaveBinary(Ref<ScriptClass> asset) override;
	};
}
