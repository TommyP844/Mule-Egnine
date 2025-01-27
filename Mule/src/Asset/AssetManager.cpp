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

	std::vector<Ref<IAsset>> AssetManager::GetAssetsOfType(AssetType type) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mAssetTypes.find(type);
		if (iter == mAssetTypes.end())
			return {};
		return iter->second;
	}

	void AssetManager::RemoveAsset(AssetHandle handle)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mAssets.find(handle);
		if (iter == mAssets.end())
		{
			SPDLOG_WARN("Trying to remove invalid asset: {}", handle);
			return;
		}
		mAssets.erase(handle);
	}
}