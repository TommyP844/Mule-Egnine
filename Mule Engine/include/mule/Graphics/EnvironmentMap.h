#pragma once

#include "Asset/Asset.h"

#include "Ref.h"
#include "Graphics/API/TextureCube.h"

namespace Mule
{
	class EnvironmentMap : public Asset<AssetType::EnvironmentMap>
	{
	public:
		EnvironmentMap(const fs::path& filepath, AssetHandle cubeMapHandle, AssetHandle diffuseIBL, AssetHandle prefilterMap);

		AssetHandle GetCubeMapHandle() const { return mCubemap; }
		AssetHandle GetDiffuseIBLMap() const { return mDiffuseIBL; }
		AssetHandle GetPreFilterMap() const { return mPreFilterMap; }

	private:
		AssetHandle mCubemap;
		AssetHandle mDiffuseIBL;
		AssetHandle mPreFilterMap;
	};
}
