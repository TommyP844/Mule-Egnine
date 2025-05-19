#pragma once
#include "Panel/IPanel.h"

class UIElementEditorPanel : public IPanel
{
public:
	UIElementEditorPanel();

	// Inherited via IPanel
	void OnAttach() override;
	void OnUIRender(float dt) override;
	void OnEditorEvent(Ref<IEditorEvent> event) override;
	void OnEngineEvent(Ref<Mule::Event> event) override;
};

