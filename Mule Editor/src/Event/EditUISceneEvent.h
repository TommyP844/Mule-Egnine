#pragma once

#include "Mule.h"
#include "IEditorEvent.h"

class EditUISceneEvent : IEditorEvent
{
public:
	EditUISceneEvent(Mule::AssetHandle uiSceneHandle)
		:
		IEditorEvent(EditorEventType::EditUIScene),
		mUISceneHandle(uiSceneHandle)
	{
	}

	Mule::AssetHandle GetUISceneHandle() const { return mUISceneHandle; }

private:
	Mule::AssetHandle mUISceneHandle;
};
