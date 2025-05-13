#pragma once

// Engine
#include "Asset.h"
#include "Ref.h"
#include "WeakRef.h"
#include "Serializer/IAssetSerializer.h"
#include "Services/IService.h"

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

		void UpdateHandle(AssetHandle oldHandle, AssetHandle newHandle);

		// Loaders
		template<typename T, typename ... Args>
		Ref<T> RegisterLoader(Args&&... args);

		template<typename T>
		WeakRef<T> GetLoader();

		template<typename T>
		Ref<T> Load(const fs::path& filepath);

		template<typename T, typename Loader, typename ... Args>
		Ref<T> Load(Args&&... args);

		template<typename T>
		void Save(AssetHandle handle);

		template<typename T>
		void Save(Ref<T> asset);

		template<typename T>
		void Insert(Ref<T> asset);

		template<typename T>
		void RegisterLoadCallback(std::function<void(WeakRef<T>)> callback);

		template<typename T>
		void RegisterUnloadCallback(std::function<void(WeakRef<T>)> callback);

		template<typename T>
		std::future<Ref<T>> LoadAsync(const fs::path& filepath);

		template<typename T>
		WeakRef<T> Get(AssetHandle);

		Ref<IAsset> GetByFilepath(const fs::path& path);

		std::vector<Ref<IAsset>> GetAssetsOfType(AssetType type) const;

		void Remove(AssetHandle handle);

		const std::unordered_map<AssetHandle, Ref<IAsset>>& GetAllAssets() const { return mAssets; }

	private:
		mutable std::mutex mMutex;
		std::unordered_map<AssetHandle, Ref<IAsset>> mAssets;
		std::map<AssetType, std::vector<Ref<IAsset>>> mAssetTypes;

		std::unordered_map<fs::path, AssetHandle> mLoadedHandles;

		std::map<AssetType, Ref<IBaseSerializer>> mLoaders;

		struct ICallback {
			virtual ~ICallback() = default;
		};

		template<typename T>
		struct Callback : ICallback {
			std::function<void(WeakRef<T>)> Callback;
		};

		std::unordered_map<std::type_index, Ref<ICallback>> mLoadCallbacks;
		std::unordered_map<std::type_index, Ref<ICallback>> mUnLoadCallbacks;
	};
}

#include "AssetManager.inl"