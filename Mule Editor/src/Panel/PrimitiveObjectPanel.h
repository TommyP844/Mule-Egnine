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
	void OnEditorEvent(Ref<IEditorEvent> event) override;
	virtual void OnEngineEvent(Ref<Mule::Event> event) override {}

private:
	void DragDropMeshSource(Mule::AssetHandle handle, const char* name);
};