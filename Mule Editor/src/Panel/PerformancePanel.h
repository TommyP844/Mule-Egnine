#pragma once

#include "IPanel.h"

class PerformancePanel : public IPanel
{
public:
	PerformancePanel();
	virtual ~PerformancePanel();

	// Inherited via IPanel
	void OnAttach() override;

	void OnUIRender(float dt) override;

	void OnEditorEvent(Ref<IEditorEvent> event) override;
	virtual void OnEngineEvent(Ref<Mule::Event> event) override {}

};
