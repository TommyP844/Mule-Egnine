#pragma once

#include "Asset/Asset.h"

#include "Ref.h"
#include "Graphics/Texture/TextureCube.h"

namespace Mule
{
	class EnvironmentMap : public Asset<AssetType::EnvironmentMap>
	{
	public:
		EnvironmentMap(const fs::path& filepath, AssetHandle cubeMapHandle);

		AssetHandle GetCubeMapHandle() const { return mCubemap; }

	private:
		AssetHandle mCubemap;
	};
}
