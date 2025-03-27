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
	if (ImGui::Begin(mName.c_str(), &mIsOpen) && mEditorContext->GetSelectedEntity())
	{
		bool entityModified = false;
		Mule::Entity e = mEditorContext->GetSelectedEntity();
			
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
			DisplayRow("ID");
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

#define ADD_COMPONENT(name, type) if(ImGui::MenuItem(name, "", nullptr, !e.HasComponent<type>())) e.AddComponent<type>();

		if (ImGui::BeginMenu("Components"))
		{
			ADD_COMPONENT("Camera", Mule::CameraComponent);
			ADD_COMPONENT("Environment", Mule::EnvironmentMapComponent);
			ADD_COMPONENT("Mesh", Mule::MeshComponent);
			if (ImGui::BeginMenu("Light"))
			{
				ADD_COMPONENT("Directional Light", Mule::DirectionalLightComponent);
				ADD_COMPONENT("Point Light", Mule::PointLightComponent);
				ADD_COMPONENT("Spot Light", Mule::SpotLightComponent);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Physics 3D"))
			{
				ADD_COMPONENT("Rigid Body 3D", Mule::RigidBody3DComponent);
				ADD_COMPONENT("Box Collider", Mule::BoxColliderComponent);
				ADD_COMPONENT("Sphere Collider", Mule::SphereColliderComponent);
				ImGui::EndMenu();
			}
			ADD_COMPONENT("Script", Mule::ScriptComponent);

			ImGui::EndMenu();
		}

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.19f, 0.19f, 1.00f));         // Normal color
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.00f));  // Hovered color
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.25f, 0.25f, 1.00f));   // Active (clicked) color

		DisplayComponent<Mule::CameraComponent>(ICON_FA_CAMERA" Camera", e, [&](Mule::CameraComponent& camera) {

			DisplayRow("Primary");
			entityModified |= ImGui::Checkbox("##CameraPrimary", &camera.Active);

			DisplayRow("Field Of View");
			float fov = camera.Camera->GetFOVDegrees();
			if (ImGui::DragFloat("##CameraFOV", &fov, 0.01f, 0.01f, 359.f, "%.2f"))
			{
				entityModified = true;
				camera.Camera->SetFOVDegrees(fov);
			}

			DisplayRow("Near Plane");
			float nearPlane = camera.Camera->GetNearPlane();
			if (ImGui::DragFloat("##CameraNearPlane", &nearPlane, 0.05f, 0.01f, camera.Camera->GetFarPlane(), "%.2f"))
			{
				entityModified = true;
				camera.Camera->SetNearPlane(nearPlane);
			}

			DisplayRow("Far Plane");
			float farPlane = camera.Camera->GetFarPlane();
			if (ImGui::DragFloat("##CameraFarPlane", &farPlane, 0.05f, camera.Camera->GetNearPlane(), 0.f, "%.2f"))
			{
				entityModified = true;
				camera.Camera->SetFarPlane(farPlane);
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

		DisplayComponent<Mule::RigidBody3DComponent>("Rigid Body 3D", e, [&](Mule::RigidBody3DComponent& rigidBody3d) {

			DisplayRow("Mass");
			ImGui::DragFloat("##Mass", &rigidBody3d.Mass, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			const char* bodyTypes[] = {
				"Dynamic",
				"Static",
				"Kinematic"
			};

			const char* preview = bodyTypes[rigidBody3d.BodyType];
			DisplayRow("Body Type");
			if (ImGui::BeginCombo("##BodyType", preview))
			{
				for (uint32_t i = 0; i < IM_ARRAYSIZE(bodyTypes); i++)
				{
					bool selected = rigidBody3d.BodyType == i;
					if (ImGui::Selectable(bodyTypes[i], selected))
					{
						rigidBody3d.BodyType = (Mule::BodyType)i;
						preview = bodyTypes[i];
					}
				}
				ImGui::EndCombo();
			}

			});

		DisplayComponent<Mule::BoxColliderComponent>("Box Collider", e, [&](Mule::BoxColliderComponent& box) {

			DisplayRow("Extent");
			ImGui::InputFloat3("##Extent", &box.Extent[0]);

			DisplayRow("Trigger");
			ImGui::Checkbox("##Trigger", &box.Trigger);

			DisplayRow("Offset");
			ImGui::InputFloat3("##Offset", &box.Offset[0]);

			});

		DisplayComponent<Mule::SphereColliderComponent>("Sphere Collider", e, [&](Mule::SphereColliderComponent& sphere) {
			
			DisplayRow("Radius");
			ImGui::DragFloat("##Radius", &sphere.Radius, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			DisplayRow("Trigger");
			ImGui::Checkbox("##Trigger", &sphere.Trigger);

			DisplayRow("Offset");
			ImGui::InputFloat3("##Offset", &sphere.Offset[0]);

			});

		DisplayComponent<Mule::ScriptComponent>(ICON_FA_PEN" Script", e, [&](Mule::ScriptComponent& script) {

			WeakRef<Mule::ScriptContext> scriptContext = mEngineContext->GetScriptContext();
			WeakRef<Mule::ScriptInstance> scriptInstance = scriptContext->GetScriptInstance(script.Handle);

			std::string scriptName = "(Null)";
			if(scriptInstance)
				scriptName = scriptInstance->GetName();

			DisplayRow("Script");
			ImGui::BeginDisabled();
			ImGui::InputText("##ScriptName", scriptName.data(), scriptName.size());
			ImGui::EndDisabled();

			ImGuiExtension::DragDropFile ddf;
			if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
			{
				if (ddf.AssetType == Mule::AssetType::Script)
				{
					// TODO: descroy current object if exists

					auto scriptClass = mEngineContext->GetAsset<Mule::ScriptClass>(ddf.AssetHandle);
					auto scriptContext = mEngineContext->GetScriptContext();

					fs::path p = scriptClass->Name();
					std::string scriptClassName = p.filename().replace_extension().string();
					script.Handle = scriptContext->CreateInstance(scriptClassName, e.Guid());
				}
			}

			if (scriptInstance)
			{
				const Mule::ScriptType& scriptType = scriptContext->GetType(scriptInstance->GetName());

				for (const auto& [name, field] : scriptType.GetFields())
				{
					if (field.Accessibility != Mule::FieldAccessibility::Public)
						continue;

					DisplayRow(field.Name.c_str());					

					std::string fieldName = "##" + field.Name;

					switch (field.Type)
					{
					case Mule::ScriptFieldType::Bool:
					{
						bool value = scriptInstance->GetFieldValue<bool>(field.Name);
						if (ImGui::Checkbox(fieldName.c_str(), &value))
						{
							scriptInstance->SetFieldValue(field.Name, value);
						}
					}
						break;
					case Mule::ScriptFieldType::UInt16:
					{
						auto value = scriptInstance->GetFieldValue<uint16_t>(field.Name);
						if (ImGui::InputScalar(fieldName.c_str(), ImGuiDataType_U16, &value))
						{
							scriptInstance->SetFieldValue(field.Name, value);
						}
					}
					break;
					case Mule::ScriptFieldType::UInt32:
					{
						auto value = scriptInstance->GetFieldValue<uint32_t>(field.Name);
						if (ImGui::InputScalar(fieldName.c_str(), ImGuiDataType_U32, &value))
						{
							scriptInstance->SetFieldValue(field.Name, value);
						}
					}
					break;
					case Mule::ScriptFieldType::UInt64:
					{
						auto value = scriptInstance->GetFieldValue<uint64_t>(field.Name);
						if (ImGui::InputScalar(fieldName.c_str(), ImGuiDataType_U64, &value))
						{
							scriptInstance->SetFieldValue(field.Name, value);
						}
					}
					break;
					case Mule::ScriptFieldType::Int16:
					{
						auto value = scriptInstance->GetFieldValue<int16_t>(field.Name);
						if (ImGui::InputScalar(fieldName.c_str(), ImGuiDataType_S16, &value))
						{
							scriptInstance->SetFieldValue(field.Name, value);
						}
					}
					break;
					case Mule::ScriptFieldType::Int32:
					{
						auto value = scriptInstance->GetFieldValue<int32_t>(field.Name);
						if (ImGui::InputScalar(fieldName.c_str(), ImGuiDataType_S32, &value))
						{
							scriptInstance->SetFieldValue(field.Name, value);
						}
					}
					break;
					case Mule::ScriptFieldType::Int64:
					{
						auto value = scriptInstance->GetFieldValue<int64_t>(field.Name);
						if (ImGui::InputScalar(fieldName.c_str(), ImGuiDataType_S64, &value))
						{
							scriptInstance->SetFieldValue(field.Name, value);
						}
					}
					break;
					case Mule::ScriptFieldType::Float:
					{
						float value = scriptInstance->GetFieldValue<float>(field.Name);
						if (ImGui::InputFloat(fieldName.c_str(), &value))
						{
							scriptInstance->SetFieldValue(field.Name, static_cast<float>(value));
						}
					}
					break;
					case Mule::ScriptFieldType::Double:
					{
						double value = scriptInstance->GetFieldValue<double>(field.Name);
						if (ImGui::InputDouble(fieldName.c_str(), &value))
						{
							scriptInstance->SetFieldValue(field.Name, static_cast<double>(value));
						}
					}
					break;


					default:
						break;
					}

				}
			}

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
