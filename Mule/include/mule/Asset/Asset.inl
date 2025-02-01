#pragma once

#include "Asset.h"

namespace Mule
{
	template<AssetType Type>
	inline Mule::Asset<Type>::Asset(AssetHandle handle, const fs::path& path)
		:
		IAsset(Type, std::string(), path, handle)
	{
		mName = path.filename().string();
	}

	template<AssetType Type>
	inline Asset<Type>::Asset(const std::string& name)
		:
		IAsset(Type, name, fs::path())
	{
	}

	template<AssetType Type>
	inline Asset<Type>::Asset(const fs::path& path)
		:
		IAsset(Type, std::string(), path)
	{
		mName = path.filename().string();
	}

	template<AssetType Type>
	inline Asset<Type>::Asset()
		:
		IAsset(Type, std::string(), fs::path())
	{
	}
}