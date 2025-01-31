#include "AssetManagerPanel.h"

AssetManagerPanel::AssetManagerPanel()
	:
	IPanel("Asset Manager")
{
}

AssetManagerPanel::~AssetManagerPanel()
{
}

void AssetManagerPanel::OnAttach()
{
}

void AssetManagerPanel::OnUIRender(float dt)
{
	if (!mIsOpen) return;

	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		Ref<Mule::AssetManager> assetManager = mEngineContext->GetAssetManager();
		if (ImGui::BeginTabBar("Asset Types"))
		{
			if (ImGui::BeginTabItem("Textures"))
			{
				uint32_t textureIndex = 0;
				for (Ref<Mule::ITexture> texture : assetManager->GetAssetsOfType(Mule::AssetType::Texture))
				{

					if (texture->GetWidth() > 0 && texture->GetHeight() > 0 && texture->GetDepth() == 1)
					{
						Ref<Mule::Texture2D> texture2d = texture;
						ImGui::Image(texture2d->GetImGuiID(), { 128, 128 });
						ImGui::SameLine();
					}	
					if (ImGui::BeginChild(("TexInfo" + std::to_string(textureIndex++)).c_str(), {ImGui::GetContentRegionAvail().x, 0.f}))
					{
						fs::path relativePath = fs::relative(texture->FilePath(), mEditorState->mAssetsPath);
						ImGui::Text("Name: %s", texture->Name().c_str());

						if (texture->GetWidth() > 0 && texture->GetHeight() == 1 && texture->GetDepth() == 1)
							ImGui::Text("Size: %i", texture->GetWidth());
						if (texture->GetWidth() > 0 && texture->GetHeight() > 1 && texture->GetDepth() == 1)
							ImGui::Text("Size: %i x %i", texture->GetWidth(), texture->GetHeight());
						else
							ImGui::Text("Size: %i x %i x %i", texture->GetWidth(), texture->GetHeight(), texture->GetDepth());

						ImGui::Text("Format: %s", Mule::GetTextureFormatName(texture->GetFormat()).c_str());
					}
					ImGui::EndChild();

					ImGui::Separator();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Scenes"))
			{
				for (Ref<Mule::Scene> scene : assetManager->GetAssetsOfType(Mule::AssetType::Scene))
				{
					ImGui::Text("Name: %s", scene->Name().c_str());
					ImGui::Separator();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Models"))
			{
				for (Ref<Mule::Model> model : assetManager->GetAssetsOfType(Mule::AssetType::Model))
				{
					fs::path relativePath = fs::relative(model->FilePath(), mEditorState->mAssetsPath);
					ImGui::Text("Name: %s", model->Name().c_str());
					ImGui::Text("Path: %s", model->FilePath().string().c_str());

					ImGui::Separator();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Audio"))
			{

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}
