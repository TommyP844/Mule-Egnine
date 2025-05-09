#pragma once

#include "Context/EditorContext.h"

#include "Mule.h"

#include <map>
#include <set>
#include <atomic>

class ThumbnailManager
{
public:
	ThumbnailManager(WeakRef<Mule::EngineContext> context, WeakRef<EditorContext> editorContext);
	~ThumbnailManager();

	WeakRef<Mule::Texture2D> GetThumbnail(Mule::AssetHandle handle);
private:
	WeakRef<Mule::Texture2D> mLoadingImage;
};

