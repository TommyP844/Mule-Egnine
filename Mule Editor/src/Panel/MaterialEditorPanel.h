#pragma once
#include "IPanel.h"
class MaterialEditorPanel : public IPanel
{
public:

	MaterialEditorPanel()
		:
		IPanel("Material Editor")
	{}
	virtual ~MaterialEditorPanel(){}
	// Inherited via IPanel
	virtual void OnAttach() override;
	virtual void OnUIRender(float dt) override;
	virtual void OnEvent(Ref<IEditorEvent> event) override;

	void SetMaterial(Mule::AssetHandle materialHandle);

private:
	WeakRef<Mule::Texture2D> mBlackTexture;
	WeakRef<Mule::Material> mMaterial;

	bool DisplayTexture(const char* name, Mule::AssetHandle& textureHandle);

};

