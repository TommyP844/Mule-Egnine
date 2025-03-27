
#include "Graphics/EnvironmentMap.h"

namespace Mule
{
	EnvironmentMap::EnvironmentMap(const fs::path& filepath, AssetHandle cubeMapHandle, AssetHandle brdfLut, AssetHandle diffuseIBL, AssetHandle prefilterMap)
		:
		Asset(filepath),
		mCubemap(cubeMapHandle),
		mDiffuseIBL(diffuseIBL),
		mBRDFLutMap(brdfLut),
		mPreFilterMap(prefilterMap)
	{
	}
}