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

		const float offset = 150.f;

		ImGui::Text("Exposure");
		ImGui::SameLine(offset); ImGui::PushItemWidth(200.f);
		ImGui::DragFloat("##Exposure", &settings.Exposure, 0.01f, 0.01f, 10.f);

		ImGui::Text("Gamma");
		ImGui::SameLine(offset); ImGui::PushItemWidth(200.f);
		ImGui::DragFloat("##Gamma", &settings.Gamma, 0.01f, 0.01f, 10.f);

		ImGui::Text("Enable Shadows");
		ImGui::SameLine(offset); ImGui::PushItemWidth(200.f);
		ImGui::Checkbox("##ShadowEnable", &settings.EnableShadows);

		ImGui::SeparatorText("Debug");

		ImGui::Text("View Cascades");
		ImGui::SameLine(offset); ImGui::PushItemWidth(200.f);
		ImGui::Checkbox("##ViewCascades", &settings.ViewCascadedShadowMaps);

		if (settings.ViewCascadedShadowMaps)
		{
			ImGui::Text("Cascade Index");
			ImGui::SameLine(offset); ImGui::PushItemWidth(200.f);
			uint32_t max = mEngineContext->GetSceneRenderer()->GetShadowPass()->GetCascadeCount() - 1;
			int index = settings.CascadeIndex;
			ImGui::DragInt("##CascadeIndex", &index, 9.01f, 0, max, "%d");
			settings.CascadeIndex = index;
		}
	}
	ImGui::End();
}

void SceneRendererSettingsPanel::OnEvent(Ref<IEditorEvent> event)
{
}
