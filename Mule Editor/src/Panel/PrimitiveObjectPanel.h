#pragma once

#include "IPanel.h"

class PrimitiveObjectPanel : public IPanel
{
public:
	PrimitiveObjectPanel();
	virtual ~PrimitiveObjectPanel();

	// Inherited via IPanel
	void OnAttach() override;
	void OnUIRender(float dt) override;
	void OnEvent(Ref<IEditorEvent> event) override;

private:
	void DragDropMeshSource(Mule::AssetHandle handle, const char* name);
};