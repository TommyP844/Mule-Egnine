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

		const char* framebuffers[] = {
			"Default",
			"Shadow Map"
		};

		ImGui::Text("Framebuffer");
		ImGui::SameLine(offset); ImGui::PushItemWidth(200.f);

		static const char* preview = framebuffers[0];
		if (ImGui::BeginCombo("##FB", preview))
		{
			for (int i = 0; i < IM_ARRAYSIZE(framebuffers); i++)
			{
				bool selected = preview == framebuffers[i];
				if (ImGui::Selectable(framebuffers[i], &selected))
				{
					preview = framebuffers[i];
				}
			}
			ImGui::EndCombo();
		}
	}
	ImGui::End();
}

void SceneRendererSettingsPanel::OnEvent(Ref<IEditorEvent> event)
{
}
