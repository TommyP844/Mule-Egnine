#pragma once

#include "Mule.h"
#include "IEditorEvent.h"

class EditMaterialEvent : public IEditorEvent
{
public:
	EditMaterialEvent(Mule::AssetHandle materialHandle)
		:
		IEditorEvent(EditorEventType::EditMaterial),
		mMaterialHandle(materialHandle)
	{ }

	Mule::AssetHandle GetMaterialHandle() const { return mMaterialHandle; }

private:
	Mule::AssetHandle mMaterialHandle;
};