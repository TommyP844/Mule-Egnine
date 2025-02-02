#include "EngineContext.h"

// Asset Loaders
#include "Asset/Loader/ModelLoader.h"
#include "Asset/Loader/SceneLoader.h"
#include "Asset/Loader/TextureLoader.h"
#include "Asset/Loader/EnvironmentMapLoader.h"

namespace Mule
{
	EngineContext::EngineContext(EngineContextDescription& description)
	{
		mWindow = MakeRef<Window>(description.WindowName);

		description.GraphicsDescription.Window = mWindow;
		mGraphicsContext = MakeRef<GraphicsContext>(description.GraphicsDescription);

		mImguiContext = MakeRef<ImGuiContext>(mGraphicsContext);

		mAssetManager = MakeRef<AssetManager>();
		mAssetManager->RegisterLoader<SceneLoader>();
		mAssetManager->RegisterLoader<EnvironmentMapLoader>(mGraphicsContext, mAssetManager);
		mAssetManager->RegisterLoader<ModelLoader>(mGraphicsContext, mAssetManager);
		mAssetManager->RegisterLoader<TextureLoader>(mGraphicsContext);

		mSceneRenderer = MakeRef<SceneRenderer>(mGraphicsContext, mAssetManager);
	}

	EngineContext::~EngineContext()
	{
		mImguiContext = nullptr;
		mSceneRenderer = nullptr;
		mAssetManager = nullptr;
		mGraphicsContext = nullptr;
	}
}