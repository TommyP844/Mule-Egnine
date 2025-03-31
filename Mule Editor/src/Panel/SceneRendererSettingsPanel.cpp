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
	}
	ImGui::End();
}

void SceneRendererSettingsPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
}
