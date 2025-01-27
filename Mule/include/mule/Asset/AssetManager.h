#pragma once

// Engine
#include "Asset.h"
#include "Ref.h"
#include "WeakRef.h"
#include "Loader/AssetLoader.h"

// Submodules

// STB
#include <unordered_map>
#include <map>
#include <future>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

namespace Mule
{
	class AssetManager
	{
	public:
		AssetManager();
		~AssetManager();
		AssetManager(const AssetManager&) = delete;

		// Loaders
		template<typename T>
		Ref<T> RegisterLoader();

		template<typename T>
		Ref<T> LoadAsset(const fs::path& filepath);

		template<typename T>
		void SaveAssetText(AssetHandle handle);

		template<typename T>
		void SaveAssetText(Ref<T> asset);

		template<typename T>
		void InsertAsset(Ref<T> asset);

		template<typename T>
		Ref<T> FindAsset(const fs::path& filepath);

		template<typename T>
		std::future<Ref<T>> LoadAssetAsync(const fs::path& filepath);

		template<typename T>
		WeakRef<T> GetAsset(AssetHandle);

		std::vector<Ref<IAsset>> GetAssetsOfType(AssetType type) const;

		void RemoveAsset(AssetHandle handle);

	private:
		mutable std::mutex mMutex;
		std::unordered_map<AssetHandle, Ref<IAsset>> mAssets;
		std::map<AssetType, std::vector<Ref<IAsset>>> mAssetTypes;

		std::map<AssetType, Ref<IBaseLoader>> mLoaders;
	};
}

#include "AssetManager.inl"