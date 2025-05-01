#pragma once

#include "IPanel.h"

class EnvironmentMapGeneratorPanel : public IPanel
{
public:
	EnvironmentMapGeneratorPanel();
	~EnvironmentMapGeneratorPanel() = default;

	// Inherited via IPanel
	void OnAttach() override;

	void OnUIRender(float dt) override;

	void OnEditorEvent(Ref<IEditorEvent> event) override;

	void OnEngineEvent(Ref<Mule::Event> event) override;

private:
	enum EnvironmentMapType
	{
		HDR,
		Cubemap
	};

	WeakRef<Mule::Texture2D> mBlackTexture;

	fs::path mSavePath;
	char mAssetName[260] = { 0 };
	float mAxisWidth = 1024.f;
	bool mCanGenerate = false;
	std::vector<ImTextureID> mTextures;
	std::vector<Mule::AssetHandle> mTextureHandles;
	EnvironmentMapType mGeneratorType = EnvironmentMapType::HDR;

	void GenerateEnvMapFromHDR();
	void GenerateEnvMapFromCube();
	void DragDropTexture(uint32_t index);
	void Generate();
	void ClearSelections();

};
