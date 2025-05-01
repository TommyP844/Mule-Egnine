#include "EnvironmentMapGeneratorPanel.h"
#include "ImGuiExtension.h"

#include <nfd.h>

EnvironmentMapGeneratorPanel::EnvironmentMapGeneratorPanel()
	:
	IPanel("Environment Map Generator")
{
}

void EnvironmentMapGeneratorPanel::OnAttach()
{
	auto assetManager = mEngineContext->GetAssetManager();
	mBlackTexture = assetManager->GetAsset<Mule::Texture2D>(MULE_BLACK_TEXTURE_HANDLE);

	mTextureHandles.resize(6);
	mTextures.resize(6);

	ClearSelections();

	memset(mAssetName, 0, 260);
}

void EnvironmentMapGeneratorPanel::OnUIRender(float dt)
{
	if (!mIsOpen)
		return;

	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		const char* options[] = {
			"HDR", 
			"Cubemap"
		};
		const char* empty = "";
		static uint32_t selectedIndex = 0;

		float width = ImGui::GetContentRegionAvail().x;
		static float textWidth = ImGui::CalcTextSize("Environment Map Type").x;

		ImGui::Text("Environment Map Type");

		const float minSeperation = 20.f;
		float comboWidth = width - textWidth - minSeperation;
		comboWidth = std::min(comboWidth, 100.f);
		float seperation = width - textWidth - comboWidth;

		ImGui::SameLine(0.f, seperation);
		ImGui::PushItemWidth(comboWidth);
		if (ImGui::BeginCombo("##Type", options[selectedIndex]))
		{
			for (int i = 0; i < IM_ARRAYSIZE(options); i++)
			{
				bool isSelected = (i == selectedIndex);
				if (ImGui::Selectable(options[i], isSelected))
				{
					selectedIndex = i;
					mGeneratorType = static_cast<EnvironmentMapType>(i);
					ClearSelections();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Text("Axis Width");
		ImGui::SameLine();
		ImGui::DragFloat("##axisWidth", &mAxisWidth, 1.f, 1.f, 4096.f, "%.f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Text("Path: %s", mSavePath.string().c_str());
		ImGui::SameLine();
		if (ImGui::Button("..."))
		{
			auto assetPath = mEditorContext->GetAssetsPath();
			std::string pathStr = assetPath.string();

			nfdchar_t* output = NULL;
			NFD_PickFolder(pathStr.c_str(), &output);
			mSavePath = output;

		}

		ImGui::Text("Name");
		ImGui::SameLine();
		ImGui::InputText("##Name", mAssetName, 260);

		ImGui::BeginDisabled(!mCanGenerate);
		if (ImGui::Button("Generate"))
		{
			Generate();
		}
		ImGui::EndDisabled();

		ImGui::Separator();

		switch (mGeneratorType)
		{
		case HDR:
			GenerateEnvMapFromHDR();
			break;
		case Cubemap:
			GenerateEnvMapFromCube();
			break;
		}
	}
	ImGui::End();
}

void EnvironmentMapGeneratorPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
}

void EnvironmentMapGeneratorPanel::OnEngineEvent(Ref<Mule::Event> event)
{
}

void EnvironmentMapGeneratorPanel::GenerateEnvMapFromHDR()
{
	DragDropTexture(0);
}

void EnvironmentMapGeneratorPanel::GenerateEnvMapFromCube()
{
	{
		ImGui::Text("Up (+Y)");
		DragDropTexture(0);
	}

	{
		ImGui::Text("Down (-Y)");
		DragDropTexture(1);
	}

	{
		ImGui::Text("Left (-X)");
		DragDropTexture(2);
	}

	{
		ImGui::Text("Right (+X)");
		DragDropTexture(3);
	}

	{
		ImGui::Text("Front (-Z)");
		DragDropTexture(4);
	}

	{
		ImGui::Text("Back (+Z)");
		DragDropTexture(5);
	}

}

void EnvironmentMapGeneratorPanel::DragDropTexture(uint32_t index)
{
	ImGui::Image(mTextures[index], ImVec2(100.f, 100.f), { 0, 0 }, { 1, 1 }, { 1, 1, 1, 1 }, {0, 0.75, 0.1, 1.0});
	ImGuiExtension::DragDropFile ddf;
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
	{
		if (ddf.AssetType == Mule::AssetType::Texture)
		{
			auto assetManager = mEngineContext->GetAssetManager();
			auto texture = assetManager->GetAsset<Mule::Texture2D>(ddf.AssetHandle);
			if (texture)
			{
				mTextures[index] = texture->GetImGuiID();
				mTextureHandles[index] = ddf.AssetHandle;

				if (mGeneratorType == EnvironmentMapType::HDR)
				{
					mCanGenerate = true;
				}
				else if (mGeneratorType == EnvironmentMapType::Cubemap)
				{
					mCanGenerate = true;
					for (auto handle : mTextureHandles)
					{
						mCanGenerate &= (bool)handle;
					}
				}
			}
		}
	}
}

void EnvironmentMapGeneratorPanel::Generate()
{
	auto environmentGenerator = mEngineContext->GetServiceManager()->Get<Mule::EnvironmentMapGenerator>();
	auto assetManager = mEngineContext->GetAssetManager();
	
	// TODO: check for file extension
	mSavePath /= (mAssetName + std::string(".yml"));
	Ref<Mule::EnvironmentMap> envMap = nullptr;
	switch (mGeneratorType)
	{
	case EnvironmentMapGeneratorPanel::HDR:
		envMap = environmentGenerator->Generate(mSavePath, mTextureHandles[0], mAxisWidth);
		break;
	case EnvironmentMapGeneratorPanel::Cubemap:
		envMap = environmentGenerator->Generate(mSavePath, mTextureHandles, mAxisWidth);
		break;
	}

	if (envMap)
	{
		assetManager->SaveAssetText(envMap);

		memset(mAssetName, 0, 260);
		mSavePath = fs::path();
		ClearSelections();
	}
	else
	{
		mSavePath = mSavePath.remove_filename();
		SPDLOG_ERROR("Failed to generate environment map");
		// TODO: raise error in ui
	}	
}

void EnvironmentMapGeneratorPanel::ClearSelections()
{
	mCanGenerate = false;
	for (uint32_t i = 0; i < 6; i++)
	{
		mTextures[i] = mBlackTexture->GetImGuiID();
		mTextureHandles[i] = Mule::AssetHandle::Null();
	}
}
