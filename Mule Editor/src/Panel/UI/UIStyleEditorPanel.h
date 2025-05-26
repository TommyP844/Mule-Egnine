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


private:
	bool mIsModified;
	WeakRef<Mule::UIBaseStyle> mStyle;
	Ref<Mule::UIBaseStyle> mTempStyle; // TODO: delete once style is saved
};


