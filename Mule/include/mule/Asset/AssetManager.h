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
		void RegisterLoader();

		template<typename T>
		Ref<T> LoadAsset(const fs::path& filepath);

		template<typename T>
		std::future<Ref<T>> LoadAssetAsync(const fs::path& filepath);

		template<typename T>
		WeakRef<T> GetAsset(AssetHandle);

		void RemoveAsset(AssetHandle handle);

	private:
		std::unordered_map<AssetHandle, Ref<IAsset>> mAssets;

		std::map<AssetType, Ref<IBaseLoader>> mLoaders;
	};
}

#include "AssetManager.inl"