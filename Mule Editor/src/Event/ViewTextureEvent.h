#pragma once

#include "IEditorEvent.h"
#include "Mule.h"

class ViewTextureEvent : IEditorEvent
{
public:
	ViewTextureEvent(Mule::AssetHandle textureHandle)
		:
		IEditorEvent(EditorEventType::ViewTexture),
		mTextureHandle(textureHandle)
	{}

	Mule::AssetHandle GetTextureHandle() const { return mTextureHandle; }

private:
	Mule::AssetHandle mTextureHandle;
};
