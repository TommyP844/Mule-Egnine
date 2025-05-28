#pragma once

#include "Panel/IPanel.h"

class UIThemeEditor : public IPanel
{
public:
	UIThemeEditor();
	virtual ~UIThemeEditor();

	void OnAttach() override;
	void OnUIRender(float dt) override;
	void OnEditorEvent(Ref<IEditorEvent> event) override;
	void OnEngineEvent(Ref<Mule::Event> event) override;

	void SetTheme(WeakRef<Mule::UITheme> theme);

private:
	WeakRef<Mule::UITheme> mTheme;
	bool mIsModified;
};

