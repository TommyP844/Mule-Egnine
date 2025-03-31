#pragma once

#include "IPanel.h"
#include <ImGuizmo.h>

class SceneViewPanel : public IPanel
{
public:
	SceneViewPanel();
	~SceneViewPanel() {}

	void OnAttach() override;
	void OnUIRender(float dt) override;
	void OnEditorEvent(Ref<IEditorEvent> event) override {}
	void OnEngineEvent(Ref<Mule::Event> event) override;

private:
	float mWidth, mHeight;
	bool mShowSettings, mIsWindowFocused;
	glm::vec3 mTranslationSnap, mRotationSnap, mScaleSnap;
	ImGuizmo::OPERATION mGizmoOp;
	ImGuizmo::MODE mGizmoMode;
	float* mGizmoSnap;
	float mCameraMovementSpeed;

	WeakRef<Mule::Texture2D> mBlackImage;

	void UpdateCamera(float dt);
	void UpdateGizmos(ImVec2 cursorPos);
	void HandleDragDrop();
	void HandlePicking(ImVec2 cursorPos);
};