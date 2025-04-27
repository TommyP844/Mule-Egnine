#include "Asset/AssetManager.h"

#include <spdlog/spdlog.h>

#include <yaml-cpp/yaml.h>

#include <fstream>


namespace Mule
{
	AssetManager::AssetManager()
	{
	}

	AssetManager::~AssetManager()
	{
		SPDLOG_INFO("Shutting down asset manager");
	}

	void AssetManager::SaveRegistry(const fs::path& filepath)
	{
		YAML::Node root;

		for (const auto& [handle, asset] : mAssets)
		{
			const fs::path& path = asset->FilePath();
			if (fs::exists(path))
			{
				root[path.string()] = (uint64_t)handle;
			}
		}

		YAML::Emitter emitter;
		emitter << root;
		std::ofstream file(filepath);
		if (!file)
		{
			SPDLOG_ERROR("Failed to save asset registry to: {}", filepath.string());
			return;
		}
		file << emitter.c_str();
		file.close();
	}

	void AssetManager::LoadRegistry(const fs::path& filepath)
	{
		if (!fs::exists(filepath))
		{
			SPDLOG_ERROR("Failed to load Aset Registry from: {}", filepath.string());
			return;
		}
		YAML::Node root = YAML::LoadFile(filepath.string());
		for (YAML::const_iterator it = root.begin();it != root.end();++it) {
			std::string key = it->first.as<std::string>();
			AssetHandle value = it->second.as<uint64_t>();
			mLoadedHandles[key] = value;
		}
	}

	void AssetManager::UpdateAssetHandle(AssetHandle oldHandle, AssetHandle newHandle)
	{
		auto asset = mAssets[oldHandle];
		asset->SetHandle(newHandle);
		mAssets[newHandle] = asset;
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
			SPDLOG_WARN("Trying to remove invalid asset: {}", handle.ToString());
			return;
		}
		mAssets.erase(handle);
	}

	Ref<IAsset> AssetManager::GetAssetByFilepath(const fs::path& path)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		for (auto [handle, asset] : mAssets)
		{
			if(asset->FilePath().empty())
				continue;

			if (fs::equivalent(path, asset->FilePath()))
			{
				return asset;
			}
		}
		return nullptr;
	}
}