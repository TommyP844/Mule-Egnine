#pragma once

#include "IPanel.h"

class SceneViewPanel : public IPanel
{
public:
	SceneViewPanel();
	~SceneViewPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender(float dt) override;
	virtual void OnEvent(Ref<IEditorEvent> event) override {}

private:
	float mWidth, mHeight;
	bool mWidgetTranslation, mWidgetRotation, mWidgetScale;

	WeakRef<Mule::Texture2D> mBlackImage;

	void UpdateCamera(float dt);
	void UpdateGizmos(ImVec2 cursorPos);
	void HandleDragDrop();
};