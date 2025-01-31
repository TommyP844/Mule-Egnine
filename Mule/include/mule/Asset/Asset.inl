#pragma once

#include "Asset.h"

namespace Mule
{
	template<AssetType Type>
	inline Mule::Asset<Type>::Asset(AssetHandle handle, const fs::path& path)
		:
		mHandle(handle),
		mFilepath(path)
	{
		mName = path.filename().string();
	}

	template<AssetType Type>
	inline Asset<Type>::Asset(const std::string& name)
		:
		mName(name),
		mHandle(GenerateUUID())
	{
	}

	template<AssetType Type>
	inline Asset<Type>::Asset(const fs::path& path)
		:
		mHandle(GenerateUUID()),
		mFilepath(path)
	{
		mName = path.filename().string();
	}

	template<AssetType Type>
	inline Asset<Type>::Asset()
		:
		mHandle(GenerateUUID())
	{
	}
}