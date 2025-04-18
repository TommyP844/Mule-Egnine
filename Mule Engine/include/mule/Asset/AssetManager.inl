#pragma once

#include <spdlog/spdlog.h>
#include "AssetManager.h"

namespace Mule
{
	template<typename T, typename ... Args>
	inline Ref<T> AssetManager::RegisterLoader(Args&&... args)
	{
		auto loader = MakeRef<T>(std::forward<Args>(args)...);
		AssetType type = T::sType;
		SPDLOG_INFO("Loader redistered with AssetManager: {}", GetAssetTypeString(type));

		std::lock_guard<std::mutex> lock(mMutex);		
		mLoaders[type] = loader;
		return loader;
	}

	template<typename T>
	inline WeakRef<T> AssetManager::GetLoader()
	{
		constexpr AssetType type = T::sType;
		Ref<IAssetSerializer<T, type>> loader = mLoaders[type];
		return loader;
	}

	template<typename T>
	inline Ref<T> AssetManager::LoadAsset(const fs::path& filepath)
	{
		constexpr AssetType type = T::sType;
		Ref<IAssetSerializer<T, type>> loader = mLoaders[type];
		if (!loader)
		{
			SPDLOG_WARN("No loader registered to load file: {}", filepath.string());
			return nullptr;
		}
		Ref<T> asset = loader->Load(filepath);
		if (!asset)
			return nullptr;

		auto iter = mLoadedHandles.find(asset->FilePath());
		if (iter != mLoadedHandles.end())
		{
			asset->mHandle = iter->second;
		}

		InsertAsset<T>(asset);
		return asset;
	}

	template<typename T, typename Loader, typename ...Args>
	inline Ref<T> AssetManager::LoadAsset(Args && ...args)
	{
		constexpr AssetType type = T::sType;
		Ref<Loader> loader = mLoaders[type];
		if (!loader)
		{
			SPDLOG_WARN("No loader registered for asset type: {}", (uint32_t)type);
			return nullptr;
		}
		Ref<T> asset = loader->LoadText(std::forward<Args>(args)...);
		if (!asset)
			return nullptr;

		auto iter = mLoadedHandles.find(asset->FilePath());
		if (iter != mLoadedHandles.end())
		{
			asset->mHandle = iter->second;
		}

		InsertAsset<T>(asset);
		return asset;
	}

	template<typename T>
	inline void AssetManager::SaveAssetText(AssetHandle handle)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		constexpr AssetType type = T::sType;
		Ref<T> asset = mAssets[handle];
		SPDLOG_INFO("saving asset asset: [{}, {}]", GetAssetTypeString(type), asset->Name());
		Ref<IAssetSerializer<T, type>> loader = mLoaders[type];
		loader->Save(asset);
	}

	template<typename T>
	inline void AssetManager::SaveAssetText(Ref<T> asset)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		constexpr AssetType type = T::sType;
		SPDLOG_INFO("saving asset asset: [{}, {}]", GetAssetTypeString(type), asset->Name());
		Ref<IAssetSerializer<T, type>> loader = mLoaders[type];
		loader->SaveText(asset);
	}

	template<typename T>
	inline void AssetManager::InsertAsset(Ref<T> asset)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mAssets[asset->Handle()] = asset;
		mAssetTypes[T::sType].push_back(asset);
	}

	template<typename T>
	inline std::future<Ref<T>> AssetManager::LoadAssetAsync(const fs::path& filepath)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return std::async(std::launch::async, &AssetManager::LoadAsset<T>, this, filepath);
	}

	template<typename T>
	inline WeakRef<T> Mule::AssetManager::GetAsset(AssetHandle handle)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = mAssets.find(handle);
		if (iter == mAssets.end())
		{
			return nullptr;
		}
		return iter->second;
	}
}