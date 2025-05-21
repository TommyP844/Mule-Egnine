#pragma once

#include "Mule.h"
#include "Context/EditorContext.h"

// Panels
#include "Panel/ComponentPanel.h"
#include "Panel/ContentBrowserPanel.h"
#include "Panel/SceneHierarchyPanel.h"
#include "Panel/SceneViewPanel.h"
#include "Panel/AssetManagerPanel.h"
#include "Panel/MaterialEditorPanel.h"
#include "Panel/TextureViewerPanel.h"
#include "Panel/SceneRendererSettingsPanel.h"
#include "Panel/PrimitiveObjectPanel.h"
#include "Panel/PerformancePanel.h"
#include "Panel/EnvironmentMapGeneratorPanel.h"
#include "Panel/UI/UIEditorPanel.h"
#include "Panel/UI/UIElementEditorPanel.h"
#include "Panel/UI/UIStyleEditorPanel.h"
#include "Panel/UI/UIThemeEditor.h"


// STD
#include <future>

class EditorLayer : public Mule::ILayer
{
public:
	EditorLayer(Ref<Mule::EngineContext> context);
	virtual ~EditorLayer();

	virtual void OnEvent(Ref<Mule::Event> event) override;
	virtual void OnAttach() override;
	virtual void OnUpdate(float dt) override;
	virtual void OnUIRender(float dt) override;
	virtual void OnRender(float dt) override;
	virtual void OnDetach() override;

private:
	Ref<EditorContext> mEditorState;
	
	// Panels
	ComponentPanel mComponentPanel;
	ContentBrowserPanel mContentBrowserPanel;
	SceneHierarchyPanel mSceneHierarchyPanel;
	SceneViewPanel mSceneViewPanel;
	AssetManagerPanel mAssetManagerPanel;
	MaterialEditorPanel mMaterialEditorPanel;
	TextureViewerPanel mTextureViewerPanel;
	SceneRendererSettingsPanel mSceneRendererSettingsPanel;
	PrimitiveObjectPanel mPrimitiveObjectPanel;
	PerformancePanel mPerformancePanel;
	EnvironmentMapGeneratorPanel mEnvironmentMapGeneratorPanel;
	UIEditorPanel mUIEditorPanel;
	UIElementEditorPanel mUIElementEditorPanel;
	UIStyleEditorPanel mUIStyleEditorPanel;
	UIThemeEditor mUIThemeEditorPanel;


	bool mShowDemoWindow = false;

	// Popups
	bool mNewScenePopup = false;
	bool mNewMaterialPopup = false;
	bool mNewScriptPopup = false;
	bool mNewUIStylePopup = false;
	bool mNewUIThemePopup = false;
	bool mNewUIScenePopup = false;

};