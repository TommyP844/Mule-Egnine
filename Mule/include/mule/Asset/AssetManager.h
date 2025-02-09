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

		void SaveRegistry(const fs::path& filepath);
		void LoadRegistry(const fs::path& filepath);

		void UpdateAssetHandle(AssetHandle oldHandle, AssetHandle newHandle);

		// Loaders
		template<typename T, typename ... Args>
		Ref<T> RegisterLoader(Args&&... args);

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

		Ref<IAsset> GetAssetByFilepath(const fs::path& path);

		std::vector<Ref<IAsset>> GetAssetsOfType(AssetType type) const;

		void RemoveAsset(AssetHandle handle);

		const std::unordered_map<AssetHandle, Ref<IAsset>>& GetAllAssets() const { return mAssets; }

	private:
		mutable std::mutex mMutex;
		std::unordered_map<AssetHandle, Ref<IAsset>> mAssets;
		std::map<AssetType, std::vector<Ref<IAsset>>> mAssetTypes;

		std::unordered_map<fs::path, AssetHandle> mLoadedHandles;

		std::map<AssetType, Ref<IBaseLoader>> mLoaders;
	};
}

#include "AssetManager.inl"