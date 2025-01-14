#include "Application/ApplicationData.h"

namespace Mule
{
	ApplicationData::ApplicationData()
	{
		mAssetManager = MakeRef<AssetManager>();
		mActiveScene = MakeRef<Scene>();
	}

	ApplicationData::~ApplicationData()
	{
	}
	void ApplicationData::Shutdown()
	{
		mActiveScene = nullptr;
		mAssetManager = nullptr;
	}
	void ApplicationData::SetGraphicsContext(Ref<GraphicsContext> graphicsContext)
	{
		mGraphicsContext = graphicsContext;
	}
}