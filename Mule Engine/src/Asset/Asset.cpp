#include "Asset/Asset.h"

namespace Mule
{
	IAsset::IAsset(AssetType type, const std::string& name, const fs::path& filepath, AssetHandle handle)
		:
		mAssetType(type),
		mName(name),
		mFilepath(filepath),
		mHandle(handle)
	{
	}
}
