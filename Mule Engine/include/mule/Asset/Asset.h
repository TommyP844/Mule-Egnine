#pragma once

#include "AssetType.h"

#include <random>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Mule
{
	typedef uint64_t AssetHandle;
	constexpr uint64_t NullAssetHandle = 0;

	static AssetHandle GenerateUUID() {
		std::random_device rd;
		std::mt19937_64 rng(rd());

		uint64_t min = UINT16_MAX;
		uint64_t max = UINT64_MAX;

		std::uniform_int_distribution<uint64_t> dist(min, max);
		
		return dist(rng);
	}

	class IAsset
	{
	protected:
		IAsset(AssetType type, const std::string& name, const fs::path& filepath, AssetHandle handle = GenerateUUID());
		const AssetType mAssetType;
		fs::path mFilepath;
		std::string mName;

	public:
		virtual ~IAsset() {}
		AssetType GetType() const { return mAssetType; }
		const std::string& Name() const { return mName; }
		const fs::path& FilePath() const { return mFilepath; }
		AssetHandle Handle() const { return mHandle; }
		std::string GetTypeName() const 
		{
			return GetAssetTypeString(mAssetType);
		}

		void SetFilePath(const fs::path& filepath)
		{
			mFilepath = filepath;
			mName = filepath.filename().string();
		}

		void SetName(const std::string& name)
		{
			mName = name;
			if (!mFilepath.empty())
			{
				mFilepath = mFilepath.parent_path() / mName / mFilepath.extension();
			}
		}

		void SetHandle(AssetHandle handle)
		{
			mHandle = handle;
		}
	private:
		friend class AssetManager;
		AssetHandle mHandle;
	};

	template<AssetType Type>
	class Asset : public IAsset
	{
	public:
		virtual ~Asset() {}
		
		static constexpr AssetType sType = Type;

	protected:
		explicit Asset(AssetHandle handle, const fs::path& path);
		explicit Asset(const std::string& name);
		explicit Asset(const fs::path& path);
		explicit Asset();	
	};
}

#include "Asset.inl"