#pragma once

#include "IPanel.h"

#include "Mule.h"

class ContentBrowserPanel : public IPanel
{
public:
	ContentBrowserPanel();
	~ContentBrowserPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender() override;

private:
	Ref<Mule::Texture2D> mFolderTexture;
	Ref<Mule::Texture2D> mFileTexture;
};