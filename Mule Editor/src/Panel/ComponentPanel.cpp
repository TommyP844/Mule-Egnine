#include "ComponentPanel.h"

#include "ImGuiExtension.h"
#include <IconsFontAwesome6.h>

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
		
		Mule::TransformComponent& transform = e.GetTransformComponent();
		if (ImGui::BeginTable("TRS", 2, ImGuiTableFlags_NoClip | ImGuiTableFlags_SizingFixedFit, {ImGui::GetContentRegionAvail().x, 0.f}))
		{
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Data");

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("%llx", (uint64_t)e.Guid());
			ImGui::TableNextColumn();
			char entityName[256] = { 0 };
			memset(entityName, 0, 256);
			memcpy(entityName, e.Name().c_str(), e.Name().size());
			if (ImGui::InputText("##EntityName", entityName, 256))
			{
				e.GetComponent<Mule::MetaComponent>().Name = entityName;
			}
			ImGui::Separator();

			DisplayRow("Translation");
			ImGuiExtension::Vec3("Translation", transform.Translation);
		
			DisplayRow("Rotation");
			ImGuiExtension::Vec3("Rotation", transform.Rotation);
		
			DisplayRow("Scale");
			ImGuiExtension::Vec3("Scale", transform.Scale, glm::vec3(1.f));
		
			ImGui::EndTable();
		}
		
		
		ImGui::Separator();

		if (ImGui::BeginMenu("Components"))
		{
			if (ImGui::MenuItem("Camera", "", nullptr, !e.HasComponent<Mule::CameraComponent>())) e.AddComponent<Mule::CameraComponent>();
			if (ImGui::MenuItem("Mesh", "", nullptr, !e.HasComponent<Mule::MeshComponent>())) e.AddComponent<Mule::MeshComponent>();
			if (ImGui::MenuItem("Sky Light", "", nullptr, !e.HasComponent<Mule::SkyLightComponent>())) e.AddComponent<Mule::SkyLightComponent>();

			ImGui::EndMenu();
		}

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.19f, 0.19f, 1.00f));         // Normal color
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.00f));  // Hovered color
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.25f, 0.25f, 1.00f));   // Active (clicked) color

		DisplayComponent<Mule::CameraComponent>(ICON_FA_CAMERA" Camera", e, [&](Mule::CameraComponent& camera) {

			DisplayRow("Primary");
			ImGui::Checkbox("##CameraPrimary", &camera.Active);

			DisplayRow("Field Of View");
			float fov = camera.Camera.GetFOVDegrees();
			if (ImGui::DragFloat("##CameraFOV", &fov, 0.01f, 0.01f, 359.f, "%.2f"))
			{
				camera.Camera.SetFOVDegrees(fov);
			}

			DisplayRow("Near Plane");
			float nearPlane = camera.Camera.GetNearPlane();
			if (ImGui::DragFloat("##CameraNearPlane", &nearPlane, 0.05f, 0.01f, camera.Camera.GetFarPlane(), "%.2f"))
			{
				camera.Camera.SetNearPlane(nearPlane);
			}

			DisplayRow("Far Plane");
			float farPlane = camera.Camera.GetFarPlane();
			if (ImGui::DragFloat("##CameraFarPlane", &farPlane, 0.05f, camera.Camera.GetNearPlane(), 0.f, "%.2f"))
			{
				camera.Camera.SetFarPlane(farPlane);
			}
		});

		if (e.HasComponent<Mule::MeshCollectionComponent>())
		{
			auto& meshCollection = e.GetComponent<Mule::MeshCollectionComponent>();
			uint32_t id = 0;
			for (auto it = meshCollection.Meshes.begin(); it != meshCollection.Meshes.end();)
			{
				id++;
				std::string uuid = "##" + std::to_string(id);
				std::string label = "Mesh" + uuid;
				bool remove = false;
				bool open = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_CollapsingHeader);

				if (ImGui::BeginPopupContextItem(label.c_str()))
				{
					if (ImGui::MenuItem("Delete"))
						remove = true;
					ImGui::EndPopup();
				}

				if (open)
				{
					if (ImGui::BeginTable("name", 2, ImGuiTableFlags_NoClip | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV, { ImGui::GetContentRegionAvail().x, 0.f }))
					{
						ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableSetupColumn("Data");

						DisplayRow("Visible");
						ImGui::Checkbox(("##MeshVisible" + uuid).c_str(), &it->Visible);

						DisplayRow("Mesh");
						auto mesh = mEngineContext->GetAssetManager()->GetAsset<Mule::Mesh>(it->MeshHandle);
						if(mesh)
							ImGui::Text(mesh->Name().c_str());
						else
							ImGui::Text("(No Mesh)");

						DisplayRow("Material");
						ImGui::Text("(No Material)");
						
						ImGui::EndTable();
					}
				}

				if (remove)
					it = meshCollection.Meshes.erase(it);
				else
					it++;
			}
		}

		ImGui::PopStyleColor(3);

	}
	ImGui::End();
}

void ComponentPanel::DisplayRow(const char* name, float dataColumnWidth)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text(name);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(dataColumnWidth);
}
