#pragma once

#include "IPanel.h"

class ContentBrowserPanel : public IPanel
{
public:
	ContentBrowserPanel() : IPanel("Content Browser") {}
	~ContentBrowserPanel() {}

	virtual void OnUIRender() override;
};