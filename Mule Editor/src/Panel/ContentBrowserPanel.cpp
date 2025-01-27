#include "ContentBrowserPanel.h"

#include "ImguiUtil.h"

#include <imgui.h>
#include <IconsFontAwesome6.h>

ContentBrowserPanel::ContentBrowserPanel()
	:
	IPanel("Content Browser")
{
}

void ContentBrowserPanel::OnAttach()
{
	mFolderTexture = mEngineContext->GetAssetManager()->LoadAsset<Mule::Texture2D>("../Assets/Textures/folder.png");
	mFileTexture = mEngineContext->GetAssetManager()->LoadAsset<Mule::Texture2D>("../Assets/Textures/file.png");
	mContentBrowserPath = mEditorState->mAssetsPath;
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
			float width = ImGui::GetContentRegionAvail().x;
			for (auto dir : fs::directory_iterator(mEditorState->mAssetsPath))
			{
				if (!dir.is_directory())
					continue;
				if (ImGui::Button(dir.path().filename().string().c_str(), { width, 0.f }))
				{
					mContentBrowserPath = dir.path();
				}
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("##Files", {width - directoryWidth, 0.f }, true))
		{
			ImGui::BeginDisabled(mContentBrowserPath == mEditorState->mAssetsPath);
			if (ImGui::Button(ICON_FA_ARROW_LEFT))
			{
				mContentBrowserPath = mContentBrowserPath.parent_path();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			fs::path relativePath = fs::relative(mContentBrowserPath, mEditorState->mAssetsPath);
			if (relativePath == ".")
				relativePath = "Assets";
			else
				relativePath = "Assets" / relativePath;
			ImGui::Text((relativePath).string().c_str());

			ImGui::Separator();

			std::vector<fs::directory_entry> sortedFiles;
			std::vector<fs::directory_entry>::iterator fileIter = sortedFiles.begin();
			for (auto dir : fs::directory_iterator(mContentBrowserPath))
			{
				if (dir.is_directory())
				{
					fileIter = sortedFiles.emplace(fileIter, dir);
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
				if (dir.is_directory() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					mContentBrowserPath = dir.path();
				}

				ImGui::SameLine();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
