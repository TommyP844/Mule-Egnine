#pragma once

#include "IPanel.h"

class ComponentPanel : public IPanel
{
public:
	ComponentPanel() : IPanel("Component Panel") {}
	~ComponentPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender() override;
};