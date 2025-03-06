#pragma once

#include "IPanel.h"

class TextureViewerPanel : public IPanel
{
public:
	TextureViewerPanel() : IPanel("Texture Viewer"), mMipLevel(0), mLayer(0) {}
	virtual ~TextureViewerPanel() {}

	// Inherited via IPanel
	void OnAttach() override;

	void OnUIRender(float dt) override;

	void OnEvent(Ref<IEditorEvent> event) override;

	void SetTexture(Mule::AssetHandle textureHandle);

private:
	WeakRef<Mule::ITexture> mTexture;
	int mMipLevel, mLayer;
};