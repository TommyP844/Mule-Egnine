#include "EngineContext.h"

// Asset Loaders
#include "Asset/Loader/ModelLoader.h"
#include "Asset/Loader/SceneLoader.h"
#include "Asset/Loader/TextureLoader.h"

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
		Ref<ModelLoader> modelLoader = mAssetManager->RegisterLoader<ModelLoader>();
		Ref<TextureLoader> textureLoader = mAssetManager->RegisterLoader<TextureLoader>();
		
		modelLoader->SetGraphicsContext(mGraphicsContext);
		modelLoader->SetAssetManager(mAssetManager);
		textureLoader->SetContext(mGraphicsContext);

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