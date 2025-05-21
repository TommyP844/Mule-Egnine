#pragma once

#include "Mule.h"
#include "IEditorEvent.h"

class EditUIThemeEvent : public IEditorEvent
{
public:
	EditUIThemeEvent(Mule::AssetHandle uiThemeHandle)
		:
		IEditorEvent(EditorEventType::EditUITheme),
		mUIThemeHandle(uiThemeHandle)
	{
	}

	Mule::AssetHandle GetUIThemeHandle() const { return mUIThemeHandle; }

private:
	Mule::AssetHandle mUIThemeHandle;
};