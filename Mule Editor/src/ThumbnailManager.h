#pragma once

#include "EditorContext.h"

#include "Mule.h"

#include <map>

class ThumbnailManager
{
public:
	ThumbnailManager(WeakRef<Mule::EngineContext> context, WeakRef<EditorContext> editorContext);
	~ThumbnailManager();

	WeakRef<Mule::Texture2D> GetThumbnail(Mule::AssetHandle handle);
private:
	WeakRef<Mule::EngineContext> mEngineContext;
	WeakRef<EditorContext> mEditorContext;
	WeakRef<Mule::Texture2D> mLoadingImage;
	std::thread mThumbnailThread;
	bool mRunning;

	std::map<Mule::AssetHandle, Ref<Mule::Texture2D>> mThumbnails;
};

