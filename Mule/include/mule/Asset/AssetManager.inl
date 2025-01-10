#pragma once

#include <spdlog/spdlog.h>
#include "AssetManager.h"

namespace Mule
{
	template<typename T>
	inline void AssetManager::RegisterLoader()
	{
		AssetType type = T::sType;
		SPDLOG_INFO("Loader redistered with AssetManager: {}", GetAssetTypeString(type));
		mLoaders[type] = Ref<T>::Make();
	}

	template<typename T>
	inline Ref<T> AssetManager::LoadAsset(const fs::path& filepath)
	{
		constexpr AssetType type = T::sType;
		SPDLOG_INFO("Loading asset: [{}, {}]", GetAssetTypeString(type), filepath.string());
		Ref<IAssetLoader<T, type>> loader = mLoaders[type];
		Ref<T> asset = loader->Load(filepath);
		mAssets[asset->Handle()] = asset;
		return asset;
	}

	template<typename T>
	inline void AssetManager::SaveAssetText(AssetHandle handle)
	{
		constexpr AssetType type = T::sType;
		Ref<T> asset = mAssets[handle];
		SPDLOG_INFO("saving asset asset: [{}, {}]", GetAssetTypeString(type), asset->Name());
		Ref<IAssetLoader<T, type>> loader = mLoaders[type];
		loader->SaveText(asset);
	}

	template<typename T>
	inline void AssetManager::SaveAssetText(Ref<T> asset)
	{
		constexpr AssetType type = T::sType;
		SPDLOG_INFO("saving asset asset: [{}, {}]", GetAssetTypeString(type), asset->Name());
		Ref<IAssetLoader<T, type>> loader = mLoaders[type];
		loader->SaveText(asset);
	}

	template<typename T>
	inline void AssetManager::InsertAsset(Ref<T> asset)
	{
		mAssets[asset->Handle()] = asset;
	}

	template<typename T>
	inline std::future<Ref<T>> AssetManager::LoadAssetAsync(const fs::path& filepath)
	{
		return std::async(std::launch::async, &AssetManager::LoadAsset<T>, this, filepath);
	}

	template<typename T>
	inline WeakRef<T> Mule::AssetManager::GetAsset(AssetHandle handle)
	{
		auto iter = mAssets.find(handle);
		if (iter == mAssets.end())
		{
			SPDLOG_ERROR("Trying to get invalid asset: {}", handle);
			return nullptr;
		}
		return iter->second;
	}
}