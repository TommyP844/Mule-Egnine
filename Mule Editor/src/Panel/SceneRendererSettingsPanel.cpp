#include "SceneRendererSettingsPanel.h"

SceneRendererSettingsPanel::SceneRendererSettingsPanel()
	:
	IPanel("Render Settings")
{
}

void SceneRendererSettingsPanel::OnAttach()
{
}

void SceneRendererSettingsPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str()))
	{
		auto sceneRenderer = mEngineContext->GetSceneRenderer();
		auto& settings = sceneRenderer->GetSettings();

		ImGui::Text("Gamma");
		ImGui::SameLine(75.f);
		ImGui::DragFloat("##Gamma", &settings.Gamma, 0.01f, 0.01f, 10.f);

		ImGui::Text("Exposure");
		ImGui::SameLine(75.f);
		ImGui::DragFloat("##Exposure", &settings.Exposure, 0.01f, 0.01f, 10.f);
	}
	ImGui::End();
}

void SceneRendererSettingsPanel::OnEvent(Ref<IEditorEvent> event)
{
}
