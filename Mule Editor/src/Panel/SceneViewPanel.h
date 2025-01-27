#pragma once

#include "IPanel.h"

class SceneViewPanel : public IPanel
{
public:
	SceneViewPanel() : IPanel("Scene View") {}
	~SceneViewPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender() override;

private:
	float mWidth, mHeight;
};