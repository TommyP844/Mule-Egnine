#pragma once

#include "IPanel.h"

class SceneViewPanel : public IPanel
{
public:
	SceneViewPanel() : IPanel("Scene View") {}
	~SceneViewPanel() {}

	virtual void OnUIRender() override;
};