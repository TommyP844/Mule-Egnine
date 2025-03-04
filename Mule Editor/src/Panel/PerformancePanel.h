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

	void OnEvent(Ref<IEditorEvent> event) override;

};
