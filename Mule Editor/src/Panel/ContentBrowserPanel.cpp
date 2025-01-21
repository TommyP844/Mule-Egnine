#include "ContentBrowserPanel.h"

#include "ImguiUtil.h"

#include <imgui.h>

ContentBrowserPanel::ContentBrowserPanel()
	:
	IPanel("Content Browser")
{
	mFolderTexture = nullptr;// MakeRef<Mule::Texture2D>("../Assets/Textures/folder.png");
	mFileTexture = nullptr; //MakeRef<Mule::Texture2D>("../Assets/Textures/file.png");
}

void ContentBrowserPanel::OnUIRender()
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{

		float width = ImGui::GetContentRegionAvail().x;

		float directoryWidth = 0.2f * width;
		if (directoryWidth > 200.f) directoryWidth = 200.f;
		if (directoryWidth < 100.f) directoryWidth = 100.f; 

		if (ImGui::BeginChild("##Directories", {directoryWidth, 0.f}, true))
		{
			for (auto dir : fs::directory_iterator(mEditorState->mAssetsPath))
			{
				if (!dir.is_directory())
					continue;
				float width = ImGui::GetContentRegionAvail().x;
				ImGui::Button(dir.path().filename().string().c_str(), { width, 0.f });
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("##Files", {width - directoryWidth, 0.f }, true))
		{
			ImGui::Text("Filepath: %s", mEditorState->mAssetsPath.string().c_str());

			std::vector<fs::directory_entry> sortedFiles;
			std::vector<fs::directory_entry>::iterator fileIter = sortedFiles.begin();
			for (auto dir : fs::directory_iterator(mEditorState->mAssetsPath))
			{
				if (dir.is_directory())
				{
					sortedFiles.emplace(fileIter, dir);
				}
				else
				{
					sortedFiles.push_back(dir);
				}
			}

			for (auto dir : sortedFiles)
			{
				std::string name;
				ImTextureID texId = 0;
				if (dir.is_directory())
				{
					name = dir.path().string();
					name = name.substr(name.find_last_of('\\') + 1);
					texId = mFolderTexture->GetImGuiID();
				}
				else
				{
					name = dir.path().filename().string();
					texId = mFileTexture->GetImGuiID();
				}

				ImguiUtil::File(name, texId);


				ImGui::SameLine();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
