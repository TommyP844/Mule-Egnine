#pragma once

#include "Panel/IPanel.h"

#include "Mule.h"

class UIStyleEditorPanel : public IPanel
{
public:
	UIStyleEditorPanel();
	// Inherited via IPanel
	void OnAttach() override;
	void OnUIRender(float dt) override;
	void OnEditorEvent(Ref<IEditorEvent> event) override;
	void OnEngineEvent(Ref<Mule::Event> event) override;

	void SetStyle(Ref<Mule::UIStyle> style);

private:
	WeakRef<Mule::UIStyle> mStyle;
	bool mIsModified;
};

