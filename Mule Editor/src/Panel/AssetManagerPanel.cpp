#include "AssetManagerPanel.h"

#include "ImGuiExtension.h"
#include "IconsFontAwesome6.h"

// Events
#include "Event/EditMaterialEvent.h"
#include "Event/ViewTextureEvent.h"

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
		WeakRef<Mule::IAsset> searchedAsset = nullptr;

		ImGui::Text("Search Handle"); ImGui::SameLine();
		static char searchBuffer[256] = { 0 };
		if (ImGui::InputText("##SearchAsset", searchBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Mule::AssetHandle handle = std::stoull(searchBuffer);
			searchedAsset = assetManager->GetAsset<Mule::IAsset>(handle);
		}


		if (searchedAsset)
		{
			ImGui::SameLine();
			ImGuiExtension::PushRedButtonStyle();
			if (ImGui::Button(ICON_FA_X))
			{
				searchedAsset = nullptr;
			}
			ImGuiExtension::PopRedButtonStyle();
		}

		ImGui::Separator();

		if (ImGui::BeginChild("Assets"))
		{
			ImGui::BeginDisabled();

			if (searchedAsset)
			{
				DisplayAsset(searchedAsset);
			}
			else
			{
				for (const auto& [handle, asset] : assetManager->GetAllAssets())
				{
					DisplayAsset(asset);
					ImGui::Separator();
				}
			}
			ImGui::EndDisabled();
		}
		ImGui::EndChild();
		
	}
	ImGui::End();
}

void AssetManagerPanel::DisplayAsset(WeakRef<Mule::IAsset> asset)
{
	const float requestedOffset = 100.f;
	std::string name = asset->Name();
	std::string guid = std::to_string(asset->Handle());
	std::string path = fs::relative(asset->FilePath(), mEditorContext->mAssetsPath).string();
	std::string type = asset->GetTypeName();
	ImGui::PushID(asset->Handle());

	ImGui::Text("Name"); 
	ImGui::SameLine(requestedOffset);
	ImGui::InputText("##Name", name.data(), name.size()); ImGui::SameLine();
	
	ImGui::EndDisabled();
	if (ImGui::Button("View"))
	{
		switch (asset->GetType())
		{
		case Mule::AssetType::Material:
		{
			Ref<EditMaterialEvent> event = MakeRef<EditMaterialEvent>(asset->Handle());
			mEditorContext->PushEvent(event);
		}
		break;
		case Mule::AssetType::Texture:
		{
			Ref<ViewTextureEvent> event = MakeRef<ViewTextureEvent>(asset->Handle());
			mEditorContext->PushEvent(event);
		}
		break;
		default:
			break;
		}
	}
	ImGui::BeginDisabled();
	
	ImGui::Text("GUID");  
	ImGui::SameLine(requestedOffset);
	ImGui::InputText("##Guid", guid.data(), guid.size());
	
	ImGui::Text("Filepath"); 
	ImGui::SameLine(requestedOffset);
	ImGui::InputText("##Filepath", path.data(), path.size());
	
	ImGui::Text("Asset Type");
	ImGui::SameLine(requestedOffset);
	ImGui::InputText("##Type", type.data(), type.size());
	
	ImGui::PopID();
}
