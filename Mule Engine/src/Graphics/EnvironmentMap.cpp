
#include "Graphics/EnvironmentMap.h"

namespace Mule
{
	EnvironmentMap::EnvironmentMap(const fs::path& filepath, AssetHandle cubeMapHandle, AssetHandle diffuseIBL, AssetHandle prefilterMap)
		:
		Asset(filepath),
		mCubemap(cubeMapHandle),
		mDiffuseIBL(diffuseIBL),
		mPreFilterMap(prefilterMap)
	{
	}
}