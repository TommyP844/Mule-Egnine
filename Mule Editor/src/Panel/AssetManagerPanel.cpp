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

void AssetManagerPanel::OnUIRender()
{
	if (!mIsOpen) return;

	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		Ref<Mule::AssetManager> assetManager = mEngineContext->GetAssetManager();
		if (ImGui::BeginTabBar("Asset Types"))
		{
			if (ImGui::BeginTabItem("Textures"))
			{
				for (Ref<Mule::ITexture> texture : assetManager->GetAssetsOfType(Mule::AssetType::Texture))
				{
					fs::path relativePath = fs::relative(texture->FilePath(), mEditorState->mAssetsPath);
					ImGui::Text("Name: %s", texture->Name().c_str());
					ImGui::Text("Filepath: %s", relativePath.string().c_str());

					if (texture->GetWidth() > 0 && texture->GetHeight() == 1 && texture->GetDepth() == 1)
						ImGui::Text("Dimension: %i", texture->GetWidth());
					if (texture->GetWidth() > 0 && texture->GetHeight() > 1 && texture->GetDepth() == 1)
						ImGui::Text("Dimension: %i x %i", texture->GetWidth(), texture->GetHeight());
					else
						ImGui::Text("Dimension: %i x %i x %i", texture->GetWidth(), texture->GetHeight(), texture->GetDepth());

					ImGui::Text("Layers: %i", texture->GetLayerCount());
					ImGui::Text("Mip count: %i", texture->GetMipCount());
					ImGui::Text("Format: %s", Mule::GetTextureFormatName(texture->GetFormat()).c_str());
					ImGui::Text("Pixel Size: %i", Mule::GetFormatSize(texture->GetFormat()));

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
					ImGui::Text("Path: %s", relativePath.string().c_str());

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
