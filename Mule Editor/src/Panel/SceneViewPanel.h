#pragma once

#include "IPanel.h"

class SceneViewPanel : public IPanel
{
public:
	SceneViewPanel();
	~SceneViewPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender(float dt) override;

private:
	float mWidth, mHeight;
	bool mWidgetTranslation, mWidgetRotation, mWidgetScale;
};