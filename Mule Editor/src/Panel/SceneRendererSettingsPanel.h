#pragma once
#include "IPanel.h"
class SceneRendererSettingsPanel : public IPanel
{
public:
	SceneRendererSettingsPanel();
	~SceneRendererSettingsPanel(){}


	// Inherited via IPanel
	void OnAttach() override;

	void OnUIRender(float dt) override;

	void OnEvent(Ref<IEditorEvent> event) override;

};

