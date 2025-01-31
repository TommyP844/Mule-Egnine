#pragma once
#include "IPanel.h"

class AssetManagerPanel : public IPanel
{
public:
	AssetManagerPanel();
	~AssetManagerPanel();
	// Inherited via IPanel
	virtual void OnAttach() override;
	virtual void OnUIRender(float dt) override;

private:
};

