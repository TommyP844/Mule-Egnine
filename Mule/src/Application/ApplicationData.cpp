#include "Application/ApplicationData.h"

namespace Mule
{
	ApplicationData::ApplicationData()
	{
		mAssetManager = Ref<AssetManager>::Make();
		mActiveScene = Ref<Scene>::Make();
	}

	ApplicationData::~ApplicationData()
	{
	}
	void ApplicationData::Shutdown()
	{
		mActiveScene = nullptr;
		mAssetManager = nullptr;
	}
}