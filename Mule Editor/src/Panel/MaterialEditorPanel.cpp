#include "MaterialEditorPanel.h"

#include "ImGuiExtension.h"

#include "IconsFontAwesome6.h"

void MaterialEditorPanel::OnAttach()
{
	auto assetManager = mEngineContext->GetAssetManager();
	mBlackTexture = assetManager->Load<Mule::Texture2D>("../Assets/Textures/Black.png");
}

void MaterialEditorPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;
	auto assetManager = mEngineContext->GetAssetManager();
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		if (mMaterial == nullptr)
		{
			ImGui::Text("No Material");
		}
		else
		{
			bool modified = false;

			std::string name = mMaterial->Name();
			std::string guid = mMaterial->Handle().ToString();
			float windowWidth = ImGui::GetContentRegionAvail().x;
			const float headerOffset = 75.f;
			const float paramOffset = 170.f;

			if (!mMaterial->FilePath().empty())
			{
				if (ImGui::Button("Save"))
				{
					assetManager->Save<Mule::Material>(mMaterial->Handle());
				}
			}

			ImGui::BeginDisabled();
			
			ImGui::Text("Name"); 
			ImGui::SameLine(headerOffset);
			ImGui::PushItemWidth(windowWidth - headerOffset);
			ImGui::InputText("##MaterialName", name.data(), name.size());

			ImGui::Text("Guid");
			ImGui::SameLine(headerOffset);
			ImGui::PushItemWidth(windowWidth - headerOffset);
			ImGui::InputText("##MaterialGuid", guid.data(), guid.size());

			ImGui::EndDisabled();

			ImGui::Text("Transparent");
			ImGui::SameLine(paramOffset);
			modified |= ImGui::Checkbox("##Transparent", &mMaterial->Transparent);

			if (mMaterial->Transparent)
			{
				ImGui::Text("Transparency");
				ImGui::SameLine(paramOffset);
				ImGui::PushItemWidth(windowWidth - paramOffset);
				modified |= ImGui::DragFloat("##Transparency", &mMaterial->Transparency, 0.01f, 0.f, 1.f);
			}

			ImGui::Text("Albedo Color");
			ImGui::SameLine(paramOffset);
			modified |= ImGui::ColorEdit3("##AlbedoColor", &mMaterial->AlbedoColor[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

			ImGui::Text("Metalness Factor");
			ImGui::SameLine(paramOffset);
			ImGui::PushItemWidth(windowWidth - paramOffset);
			modified |= ImGui::DragFloat("##MetalnessFactor", &mMaterial->MetalnessFactor, 0.01f, 0.f, 1.f);

			ImGui::Text("Roughness Factor");
			ImGui::SameLine(paramOffset);
			ImGui::PushItemWidth(windowWidth - paramOffset);
			modified |= ImGui::DragFloat("##roughnessFactor", &mMaterial->RoughnessFactor, 0.01f, 0.f, 1.f);

			ImGui::Text("Ambient Occ. Factor");
			ImGui::SameLine(paramOffset);
			ImGui::PushItemWidth(windowWidth - paramOffset);
			modified |= ImGui::DragFloat("##AOFactor", &mMaterial->AOFactor, 0.01f, 0.f, 1.f);

			ImGui::Text("Texture Scale");
			ImGui::SameLine(paramOffset);
			ImGui::PushItemWidth(windowWidth - paramOffset);
			modified |= ImGui::DragFloat2("##TextureScale", &mMaterial->TextureScale[0], 0.1f, 0.1f, 0.f);

			ImGui::Separator();

			modified |= DisplayTexture("Albedo", mMaterial->AlbedoMap);
			modified |= DisplayTexture("Normal", mMaterial->NormalMap);
			modified |= DisplayTexture("Metalness", mMaterial->MetalnessMap);
			modified |= DisplayTexture("Roughness", mMaterial->RoughnessMap);
			modified |= DisplayTexture("Ambient Occulsion", mMaterial->AOMap);
			modified |= DisplayTexture("Emissive", mMaterial->EmissiveMap);
			modified |= DisplayTexture("Opacity", mMaterial->OpacityMap);

			
			if (modified)
			{
				Mule::Renderer::Get().UpdateMaterial(mMaterial);
			}
			
		}
	}
	ImGui::End();
}

void MaterialEditorPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
}

void MaterialEditorPanel::SetMaterial(Mule::AssetHandle materialHandle)
{
	auto assetManager = mEngineContext->GetAssetManager();
	mMaterial = assetManager->Get<Mule::Material>(materialHandle);
}

bool MaterialEditorPanel::DisplayTexture(const char* name, Mule::AssetHandle& textureHandle)
{
	auto assetManager = mEngineContext->GetAssetManager();
	bool modified = false;
	WeakRef<Mule::Texture2D> texture = assetManager->Get<Mule::Texture2D>(textureHandle);

	ImGui::PushID(name);

	if (ImGui::BeginChild("MaterialTexture", {0.f, 128.f}))
	{
		if (texture)
		{
			ImGui::Image(texture->GetImGuiID(), { 128, 128 });
		}
		else
		{
			ImGui::Image(mBlackTexture->GetImGuiID(), {128, 128});
		}

		if (ImGui::BeginPopupContextItem("EditImage"))
		{
			if (ImGui::MenuItem(ICON_FA_TRASH" Remove"))
			{
				textureHandle = Mule::AssetHandle::Null();
				modified = true;
			}
			ImGui::EndPopup();
		}

		ImGuiExtension::DragDropFile ddf;
		if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
		{
			if (ddf.AssetType == Mule::AssetType::Texture)
			{
				textureHandle = ddf.AssetHandle;
				modified = true;
			}
		}

		ImGui::SameLine();

		if (ImGui::BeginChild("TextureParameters"))
		{
			ImGui::Text(name);

			if (texture)
			{
				std::string guid = texture->Handle().ToString();

				ImGui::BeginDisabled();

				ImGui::Text("Guid");
				ImGui::SameLine();
				ImGui::InputText("##guid", guid.data(), guid.size());

				ImGui::Text("Size: %i x %i", texture->GetWidth(), texture->GetHeight());

				ImGui::EndDisabled();
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	ImGui::PopID();

	ImGui::Separator();

	return modified;
}
