#include "ComponentPanel.h"

#include "ImguiUtil.h"

#include "Mule.h"

void ComponentPanel::OnAttach()
{
}

void ComponentPanel::OnUIRender()
{
	if (!mIsOpen) return;
	
	ImGui::SetNextWindowSizeConstraints({ 500.f, 300.f }, {0.f, 0.f});
	if (ImGui::Begin(mName.c_str(), &mIsOpen) && mEditorState->SelectedEntity)
	{
		Mule::Entity e = mEditorState->SelectedEntity;
		
		ImGui::Text("Name");
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(e.Name().c_str()).x);
		ImGui::Text(e.Name().c_str());

		std::string guid = std::to_string(e.Guid());
		ImGui::Text("Guid");
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(guid.c_str()).x);
		ImGui::Text(guid.c_str());

		ImGui::Separator();

		Mule::TransformComponent& transform = e.GetTransformComponent();
		if (ImGui::BeginTable("TRS", 2, ImGuiTableFlags_NoClip | ImGuiTableFlags_SizingFixedFit, {ImGui::GetContentRegionAvail().x, 0.f}))
		{
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Data");

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Translation");
			ImGui::TableNextColumn();
			ImguiUtil::Vec3("Translation", transform.Translation);
		
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Rotation");
			ImGui::TableNextColumn();
			ImguiUtil::Vec3("Rotation", transform.Rotation);
		
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Scale");
			ImGui::TableNextColumn();
			ImguiUtil::Vec3("Scale", transform.Scale, glm::vec3(1.f));
		
			ImGui::EndTable();
		}
		
		
		ImGui::Separator();

		if (ImGui::BeginMenu("Components"))
		{
			if (ImGui::MenuItem("Camera", "", nullptr, !e.HasComponent<Mule::CameraComponent>())) e.AddComponent<Mule::CameraComponent>();
			if (ImGui::MenuItem("Sky Light", "", nullptr, !e.HasComponent<Mule::SkyLightComponent>())) e.AddComponent<Mule::SkyLightComponent>();

			ImGui::EndMenu();
		}

	}
	ImGui::End();
}
