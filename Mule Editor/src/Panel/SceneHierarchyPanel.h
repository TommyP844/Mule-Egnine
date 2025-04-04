#pragma once

#include "IPanel.h"

class SceneHierarchyPanel : public IPanel
{
public:
	SceneHierarchyPanel() : IPanel("Scene") {}
	~SceneHierarchyPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender(float dt) override;
	virtual void OnEditorEvent(Ref<IEditorEvent> event) override {}
	virtual void OnEngineEvent(Ref<Mule::Event> event) override {}


private:
	Mule::Entity mEntityToOrphan;
	Mule::Entity mEntityToDelete;
	void RecurseEntities(Mule::Entity e);
	void EntityContextMenu(Mule::Entity e);
};