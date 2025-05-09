#include "ThumbnailManager.h"

ThumbnailManager::ThumbnailManager(WeakRef<Mule::EngineContext> context, WeakRef<EditorContext> editorContext)
{
	auto assetManager = context->GetAssetManager();
	mLoadingImage = assetManager->LoadAsset<Mule::Texture2D>("../Assets/Textures/Loading.png");
}

ThumbnailManager::~ThumbnailManager()
{
}

WeakRef<Mule::Texture2D> ThumbnailManager::GetThumbnail(Mule::AssetHandle handle)
{
	return mLoadingImage;
}

