#pragma once
#include "IPanel.h"
class SceneRendererSettingsPanel : public IPanel
{
public:
	SceneRendererSettingsPanel();
	~SceneRendererSettingsPanel(){}


	// Inherited via IPanel
	void OnAttach() override;
	void OnUIRender(float dt) override;
	void OnEditorEvent(Ref<IEditorEvent> event) override;
	void OnEngineEvent(Ref<Mule::Event> event) override {}

private:
	Mule::ResourceType mType;
	void DisplayRegistry(Ref<Mule::ResourceRegistry> registry);
	void DisplayRenderTargets(const Mule::ResourceHandle& handle, Ref<Mule::Texture> renderTarget, Ref<Mule::ResourceRegistry> registry);
	void DisplayUniformBuffers(const Mule::ResourceHandle& handle, Ref<Mule::UniformBuffer> uniformBuffer);
	void DisplayShaderResourceGroups(const Mule::ResourceHandle& handle, Ref<Mule::ShaderResourceGroup> srg);

};

