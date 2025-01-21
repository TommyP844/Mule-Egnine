#include "Asset/AssetManager.h"

#include <spdlog/spdlog.h>


namespace Mule
{
	AssetManager::AssetManager()
	{
	}

	AssetManager::~AssetManager()
	{
		SPDLOG_INFO("Shutting down asset manager");
	}

	void AssetManager::RemoveAsset(AssetHandle handle)
	{
		auto iter = mAssets.find(handle);
		if (iter == mAssets.end())
		{
			SPDLOG_WARN("Trying to remove invalid asset: {}", handle);
			return;
		}
		mAssets.erase(handle);
	}
}