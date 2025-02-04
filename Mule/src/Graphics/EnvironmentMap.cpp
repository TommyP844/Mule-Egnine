
#include "Graphics/EnvironmentMap.h"

namespace Mule
{
	EnvironmentMap::EnvironmentMap(const fs::path& filepath, AssetHandle cubeMapHandle)
		:
		Asset(filepath),
		mCubemap(cubeMapHandle)
	{
	}
}