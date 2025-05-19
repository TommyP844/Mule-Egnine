#pragma once

#include "Mule.h"
#include "IEditorEvent.h"

class EditUIStyleEvent : public IEditorEvent
{
public:
	EditUIStyleEvent(Mule::AssetHandle uiStyleHandle)
		:
		IEditorEvent(EditorEventType::EditUIStyle),
		mUIStyleHandle(uiStyleHandle)
	{
	}

	Mule::AssetHandle GetUIStyleHandle() const { return mUIStyleHandle; }

private:
	Mule::AssetHandle mUIStyleHandle;
};