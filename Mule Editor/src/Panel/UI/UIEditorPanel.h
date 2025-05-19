#pragma once

#include "Panel/IPanel.h"

class UIEditorPanel : public IPanel
{
public:
	UIEditorPanel();

	void OnAttach() override;

	void OnUIRender(float dt) override;

	void OnEditorEvent(Ref<IEditorEvent> event) override;

	void OnEngineEvent(Ref<Mule::Event> event) override;

private:
};