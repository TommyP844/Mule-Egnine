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
	inline Ref<T> AssetManager::Load(const fs::path& filepath)
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

		Insert<T>(asset);
		return asset;
	}

	template<typename T, typename Loader, typename ...Args>
	inline Ref<T> AssetManager::Load(Args && ...args)
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
	inline void AssetManager::Save(AssetHandle handle)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		constexpr AssetType type = T::sType;
		Ref<T> asset = mAssets[handle];
		SPDLOG_INFO("saving asset asset: [{}, {}]", GetAssetTypeString(type), asset->Name());
		Ref<IAssetSerializer<T, type>> loader = mLoaders[type];
		loader->Save(asset);
	}

	template<typename T>
	inline void AssetManager::Save(Ref<T> asset)
	{
		constexpr AssetType type = T::sType;
		SPDLOG_INFO("saving asset asset: [{}, {}]", GetAssetTypeString(type), asset->Name());
		Ref<IAssetSerializer<T, type>> loader = nullptr;
		
		{
			std::lock_guard<std::mutex> lock(mMutex);
			loader = mLoaders[type];
		}

		loader->Save(asset);		
	}

	template<typename T>
	inline void AssetManager::Insert(Ref<T> asset)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mAssets[asset->Handle()] = asset;
		mAssetTypes[T::sType].push_back(asset);
		if(!asset->mFilepath.empty())
			mLoadedHandles[asset->mFilepath] = asset->Handle();
		
		auto iter = mLoadCallbacks.find(typeid(T));
		if (iter != mLoadCallbacks.end())
		{
			Ref<Callback<T>> callback = iter->second;
			callback->Callback(asset);
		}

	}

	template<typename T>
	inline void AssetManager::RegisterLoadCallback(std::function<void(WeakRef<T>)> callback)
	{
		auto callbackContainer = MakeRef<Callback<T>>();

		callbackContainer->Callback = callback;

		mLoadCallbacks[typeid(T)] = callbackContainer;
	}

	template<typename T>
	inline void AssetManager::RegisterUnloadCallback(std::function<void(WeakRef<T>)> callback)
	{
		auto callbackContainer = MakeRef<Callback<T>>();

		callbackContainer->Callback = callback;

		mUnLoadCallbacks[typeid(T)] = callbackContainer;
	}

	template<typename T>
	inline std::future<Ref<T>> AssetManager::LoadAsync(const fs::path& filepath)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return std::async(std::launch::async, &AssetManager::Load<T>, this, filepath);
	}

	template<typename T>
	inline WeakRef<T> Mule::AssetManager::Get(AssetHandle handle)
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