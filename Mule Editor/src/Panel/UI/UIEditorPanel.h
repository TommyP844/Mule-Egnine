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

	void SetUIScene(WeakRef<Mule::UIScene> scene);

private:
	WeakRef<Mule::Texture2D> mBlackTexture;
	WeakRef<Mule::UIScene> mUIScene;
	Ref<Mule::Camera> mUIEditorCamera;
	ImVec2 mViewportSize;
	WeakRef<Mule::UIElement> mSelectedElement;
	bool mIsModified;

	void DisplayElementPanel();
	void DisplayCanvasPanel();
	void DisplayInspectorPanel();

	void DisplayElementSelection(Mule::UIElementType type);
	void DisplayUIMeasurement(const char* label, Mule::UIMeasurement& measurement, float parentSize);
};