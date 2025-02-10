#include "ThumbnailManager.h"

ThumbnailManager::ThumbnailManager(WeakRef<Mule::EngineContext> context, WeakRef<EditorContext> editorContext)
	:
	mEngineContext(context),
	mEditorContext(editorContext)
{
	mLoadingImage = mEngineContext->GetAssetManager()->LoadAsset<Mule::Texture2D>("../Assets/Textures/Loading.png");
}

ThumbnailManager::~ThumbnailManager()
{
}

WeakRef<Mule::Texture2D> ThumbnailManager::GetThumbnail(Mule::AssetHandle handle)
{
	return WeakRef<Mule::Texture2D>();
}
