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
	WeakRef<Mule::Texture2D> mBlackTexture;
	Ref<Mule::UIScene> mUIScene;
	Ref<Mule::Camera> mUIEditorCamera;
	ImVec2 mViewportSize;

	void DisplayElementPanel();
	void DisplayCanvasPanel();
	void DisplayInspectorPanel();

	void DisplayElementSelection(Mule::UIElementType type);
};