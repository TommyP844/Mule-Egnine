#include "ComponentPanel.h"

#include "ImGuiExtension.h"
#include <IconsFontAwesome6.h>

#include "Mule.h"

void ComponentPanel::OnAttach()
{
}

void ComponentPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	
	ImGui::SetNextWindowSizeConstraints({ 500.f, 300.f }, {0.f, 0.f});
	if (ImGui::Begin(mName.c_str(), &mIsOpen) && mEditorContext->SelectedEntity)
	{
		bool entityModified = false;
		Mule::Entity e = mEditorContext->SelectedEntity;
		
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
			entityModified |= ImGuiExtension::Vec3("Translation", transform.Translation);
		
			DisplayRow("Rotation");
			entityModified |= ImGuiExtension::Vec3("Rotation", transform.Rotation);
		
			DisplayRow("Scale");
			entityModified |= ImGuiExtension::Vec3("Scale", transform.Scale, glm::vec3(1.f));
		
			ImGui::EndTable();
		}
		
		
		ImGui::Separator();

		if (ImGui::BeginMenu("Components"))
		{
			if (ImGui::MenuItem("Camera", "", nullptr, !e.HasComponent<Mule::CameraComponent>())) e.AddComponent<Mule::CameraComponent>();
			if (ImGui::MenuItem("Environment Map", "", nullptr, !e.HasComponent<Mule::EnvironmentMapComponent>())) e.AddComponent<Mule::EnvironmentMapComponent>();
			if (ImGui::MenuItem("Mesh", "", nullptr, !e.HasComponent<Mule::MeshComponent>())) e.AddComponent<Mule::MeshComponent>();
			if (ImGui::BeginMenu("Light"))
			{
				if (ImGui::MenuItem("Directional Light", "", nullptr, !e.HasComponent<Mule::DirectionalLightComponent>())) e.AddComponent<Mule::DirectionalLightComponent>();
				if (ImGui::MenuItem("Point Light", "", nullptr, !e.HasComponent<Mule::PointLightComponent>())) e.AddComponent<Mule::PointLightComponent>();
				if (ImGui::MenuItem("Spot Light", "", nullptr, !e.HasComponent<Mule::SpotLightComponent>())) e.AddComponent<Mule::SpotLightComponent>();
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.19f, 0.19f, 1.00f));         // Normal color
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.00f));  // Hovered color
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.25f, 0.25f, 1.00f));   // Active (clicked) color

		DisplayComponent<Mule::CameraComponent>(ICON_FA_CAMERA" Camera", e, [&](Mule::CameraComponent& camera) {

			DisplayRow("Primary");
			entityModified |= ImGui::Checkbox("##CameraPrimary", &camera.Active);

			DisplayRow("Field Of View");
			float fov = camera.Camera.GetFOVDegrees();
			if (ImGui::DragFloat("##CameraFOV", &fov, 0.01f, 0.01f, 359.f, "%.2f"))
			{
				entityModified = true;
				camera.Camera.SetFOVDegrees(fov);
			}

			DisplayRow("Near Plane");
			float nearPlane = camera.Camera.GetNearPlane();
			if (ImGui::DragFloat("##CameraNearPlane", &nearPlane, 0.05f, 0.01f, camera.Camera.GetFarPlane(), "%.2f"))
			{
				entityModified = true;
				camera.Camera.SetNearPlane(nearPlane);
			}

			DisplayRow("Far Plane");
			float farPlane = camera.Camera.GetFarPlane();
			if (ImGui::DragFloat("##CameraFarPlane", &farPlane, 0.05f, camera.Camera.GetNearPlane(), 0.f, "%.2f"))
			{
				entityModified = true;
				camera.Camera.SetFarPlane(farPlane);
			}
		});

		DisplayComponent<Mule::DirectionalLightComponent>(ICON_FA_SUN" Directional Light", e, [&](Mule::DirectionalLightComponent& light) {

			DisplayRow("Active");
			entityModified |= ImGui::Checkbox("##Active", &light.Active);

			DisplayRow("Intensity");
			entityModified |= ImGui::DragFloat("##Intensity", &light.Intensity, 1.f, 1.f, FLT_MAX);

			DisplayRow("Color");
			entityModified |= ImGui::ColorEdit3("##Color", &light.Color[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs);

			});

		DisplayComponent<Mule::EnvironmentMapComponent>(ICON_FA_SUN" Environment Map", e, [&](Mule::EnvironmentMapComponent& light) {
			DisplayRow("Active");
			entityModified |= ImGui::Checkbox("##LightActive", &light.Active);

			DisplayRow("Radiance");
			entityModified |= ImGui::DragFloat("##Radiance", &light.Radiance, 1.f, 0.f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);

			DisplayRow("Environment Map");
			auto envMap = mEngineContext->GetAsset<Mule::EnvironmentMap>(light.EnvironmentMap);
			std::string name = "";
			if (envMap)
				name = envMap->Name();

			ImGui::BeginDisabled();
			ImGui::InputText("##EnvMap", name.data(), name.size());
			ImGui::EndDisabled();
			ImGuiExtension::DragDropFile file;
			if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, file))
			{
				if (file.AssetType == Mule::AssetType::EnvironmentMap)
				{
					light.EnvironmentMap = file.AssetHandle;
					entityModified = true;
				}
			}

			DisplayRow("Display Irradiance");
			ImGui::Checkbox("##DisplayIrradiance", &light.DisplayIrradianceMap);

			});

		DisplayComponent<Mule::MeshComponent>(ICON_FA_DIAGRAM_PROJECT" Mesh", e, [&](Mule::MeshComponent& mesh) {

			const std::string null = "";

			DisplayRow("Visible");
			entityModified |= ImGui::Checkbox("##MeshVisible", &mesh.Visible);

			ImGui::BeginDisabled();
			DisplayRow("Mesh");
			auto meshPtr = mEngineContext->GetAsset<Mule::Mesh>(mesh.MeshHandle);
			if (meshPtr)
				ImGui::InputText("##Mesh", (char*)meshPtr->Name().data(), meshPtr->Name().size());
			else
				ImGui::InputText("##Mesh", (char*)null.data(), null.size());

			DisplayRow("Material");
			auto materialPtr = mEngineContext->GetAsset<Mule::Material>(mesh.MaterialHandle);
			if (materialPtr)
				ImGui::InputText("##Material", (char*)materialPtr->Name().data(), materialPtr->Name().size());
			else
				ImGui::InputText("##Material", (char*)null.data(), null.size());
			ImGuiExtension::DragDropFile ddf;
			if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
			{
				if (ddf.AssetType == Mule::AssetType::Material)
				{
					mesh.MaterialHandle = ddf.AssetHandle;
				}
			}

			ImGui::EndDisabled();
			});

		DisplayComponent<Mule::PointLightComponent>(ICON_FA_LIGHTBULB" Point Light", e, [&](Mule::PointLightComponent& light) {
			DisplayRow("Active");
			entityModified |= ImGui::Checkbox("##LightActive", &light.Active);

			DisplayRow("Radiance");
			entityModified |= ImGui::DragFloat("##Radiance", &light.Radiance, 1.f, 0.f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);

			DisplayRow("Color");
			entityModified |= ImGui::ColorEdit3("##Color", &light.Color[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs);
			});

		DisplayComponent<Mule::SpotLightComponent>(ICON_FA_LIGHTBULB" Spot Light", e, [&](Mule::SpotLightComponent& light) {
			DisplayRow("Active");
			entityModified |= ImGui::Checkbox("##LightActive", &light.Active);

			DisplayRow("Radiance");
			entityModified |= ImGui::DragFloat("##Radiance", &light.Radiance, 1.f, 0.f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);

			DisplayRow("Angle");
			entityModified |= ImGui::DragFloat("##Angle", &light.Angle, 1.f, 0.f, 360.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

			DisplayRow("Color");
			entityModified |= ImGui::ColorEdit3("##Color", &light.Color[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs);
			});

		ImGui::PopStyleColor(3);

		if(entityModified)
			mEngineContext->GetScene()->SetModified();
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
