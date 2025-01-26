#pragma once

#include "Mule.h"
#include "Ref.h"
#include "EditorState.h"
#include "EngineContext.h"

// Panels
#include "Panel/ComponentPanel.h"
#include "Panel/ContentBrowserPanel.h"
#include "Panel/SceneHierarchyPanel.h"
#include "Panel/SceneViewPanel.h"

class EditorLayer : public Mule::ILayer
{
public:
	EditorLayer(Ref<Mule::EngineContext> context);
	virtual ~EditorLayer();

	virtual void OnAttach() override;
	virtual void OnUpdate(float dt) override;
	virtual void OnUIRender() override;
	virtual void OnDetach() override;

private:
	Ref<EditorState> mEditorState;
	
	// Panels
	ComponentPanel mComponentPanel;
	ContentBrowserPanel mContentBrowserPanel;
	SceneHierarchyPanel mSceneHierarchyPanel;
	SceneViewPanel mSceneViewPanel;

	// Popups
	bool mNewScenePopup = false;

};