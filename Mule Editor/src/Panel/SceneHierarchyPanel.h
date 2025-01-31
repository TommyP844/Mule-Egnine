#pragma once

#include "IPanel.h"

class SceneHierarchyPanel : public IPanel
{
public:
	SceneHierarchyPanel() : IPanel("Scene") {}
	~SceneHierarchyPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender(float dt) override;


private:
	Mule::Entity mEntityToOrphan;
	Mule::Entity mEntityToDelete;
	void RecurseEntities(Mule::Entity e);
	void EntityContextMenu(Mule::Entity e);
};