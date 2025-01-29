#include "ContentBrowserPanel.h"

#include "ImGuiExtension.h"

#include <imgui.h>
#include <IconsFontAwesome6.h>

// STD
#include <algorithm>
#include <regex>

ContentBrowserPanel::ContentBrowserPanel()
	:
	IPanel("Content Browser")
{
}

void ContentBrowserPanel::OnAttach()
{
	mFolderTexture = mEngineContext->GetAssetManager()->LoadAsset<Mule::Texture2D>("../Assets/Textures/folder.png");
	mFileTexture = mEngineContext->GetAssetManager()->LoadAsset<Mule::Texture2D>("../Assets/Textures/file.png");
	SetContentBrowserPath(mEditorState->mAssetsPath);
}

void ContentBrowserPanel::OnUIRender()
{
	if (!mIsOpen) return;
	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		DisplayPopups();

		float width = ImGui::GetContentRegionAvail().x;
		float directoryWidth = std::clamp(0.2f * width, 100.f, 200.f);

		ContentAssetDirBrowser(directoryWidth);
		ImGui::SameLine();
		ContentFileBrowser(width - directoryWidth);
	}
	ImGui::End();
}

void ContentBrowserPanel::ClearSearchBuffer()
{
	memset(mSearchBuffer, 0, 256);
}

void ContentBrowserPanel::SetContentBrowserPath(const fs::path& path, const std::string& filter)
{
	mContentBrowserPath = path;
	mVisibleFiles.clear();
	for (const auto& dir : fs::directory_iterator(mContentBrowserPath))
	{
		DisplayFile file;
		file.IsDirectory = dir.is_directory();
		file.FilePath = dir.path();
		file.DisplayName = dir.path().filename().string();
		file.TexId = dir.is_directory() ? mFolderTexture->GetImGuiID() : mFileTexture->GetImGuiID();
		mVisibleFiles.push_back(file);
	}

	std::sort(mVisibleFiles.begin(), mVisibleFiles.end(), [](const DisplayFile& lhs, const DisplayFile& rhs) {
		return lhs.IsDirectory > rhs.IsDirectory;
		});

	std::sort(mVisibleFiles.begin(), mVisibleFiles.end(), [](const DisplayFile& lhs, const DisplayFile& rhs) {
		return lhs.IsDirectory == rhs.IsDirectory && lhs.DisplayName < rhs.DisplayName;
		});

	if (!filter.empty())
	{
		std::string pattern = std::regex_replace(filter, std::regex("\\*"), ".*");
		std::regex searchRegex(pattern, std::regex::icase);

		for (auto it = mVisibleFiles.begin(); it != mVisibleFiles.end();)
		{
			if (!std::regex_match(it->DisplayName, searchRegex))
			{
				it = mVisibleFiles.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}

void ContentBrowserPanel::DragDropFile(const fs::path& target)
{
	fs::path dropPath;
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FOLDER_PATH, dropPath))
	{
		fs::path to = target / dropPath.filename();
		fs::rename(dropPath, to);
	}
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE_PATH, dropPath))
	{
		// TODO: update asset if needed
		fs::path to = target / dropPath.filename();
		fs::rename(dropPath, to);
	}
}

void ContentBrowserPanel::ContentAssetDirBrowser(float width)
{
	if (ImGui::BeginChild("##Directories", { width, 0.f }, true))
	{
		float width = ImGui::GetContentRegionAvail().x;
		for (auto dir : fs::directory_iterator(mEditorState->mAssetsPath))
		{
			if (!dir.is_directory())
				continue;
			if (ImGui::Button(dir.path().filename().string().c_str(), { width, 0.f }))
			{
				ClearSearchBuffer();
				SetContentBrowserPath(dir.path());
			}
			DragDropFile(dir.path());
		}
	}
	ImGui::EndChild();
}

void ContentBrowserPanel::ContentFileBrowser(float width)
{
	if (ImGui::BeginChild("##Files", { width, 0.f }, true))
	{
		ImGui::BeginDisabled(mContentBrowserPath == mEditorState->mAssetsPath);
		if (ImGui::Button(ICON_FA_ARROW_LEFT))
		{
			ClearSearchBuffer();
			SetContentBrowserPath(mContentBrowserPath.parent_path());
		}
		ImGui::EndDisabled();
		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_ROTATE_RIGHT"##RefreshContentBrowser"))
		{
			SetContentBrowserPath(mContentBrowserPath, mSearchBuffer);
		}

		ImGui::SameLine();

		ImGui::PushItemWidth(250.f);
		if (ImGui::InputText("##ContentBrowserSearch", mSearchBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			SetContentBrowserPath(mContentBrowserPath, mSearchBuffer);
		}
		ImGui::SameLine();

		ImGuiExtension::PushRedButtonStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);
		if (ImGui::Button(ICON_FA_X"##ClearContentBrowserSearch"))
		{
			ClearSearchBuffer();
			SetContentBrowserPath(mContentBrowserPath);
		}
		ImGuiExtension::PopRedButtonStyle();
		ImGui::BeginDisabled(strlen(mSearchBuffer) == 0);

		ImGui::SameLine();

		if (ImGui::Button("Search"))
		{
			SetContentBrowserPath(mContentBrowserPath, mSearchBuffer);
		}
		ImGui::EndDisabled();

		ImGui::SameLine();

		ImGui::Text("Filepath:");
		ImGui::SameLine();

		fs::path relativePath = fs::relative(mContentBrowserPath, mEditorState->mAssetsPath);
		relativePath = (relativePath == ".") ? "Assets" : "Assets" / relativePath;
		fs::path curPath = mEditorState->mAssetsPath.parent_path();
		for (auto iter = relativePath.begin(); iter != relativePath.end(); iter++)
		{
			curPath /= *iter;
			if (std::next(iter) != relativePath.end())
			{
				if (ImGui::Button((*iter).string().c_str()))
				{
					ClearSearchBuffer();
					SetContentBrowserPath(curPath);
				}
				DragDropFile(curPath);
				ImGui::SameLine();
				ImGui::Text("/");
				ImGui::SameLine();
			}
			else
			{
				ImGui::Text((*iter).string().c_str());
			}
		}
		ImGui::PopStyleVar();

		ImGui::Separator();

		if (ImGui::BeginChild("##ScrollableFileRegion", ImGui::GetContentRegionAvail()))
		{
			if (ImGui::BeginPopupContextWindow("##ScrollableFileRegion"))
			{
				if (ImGui::MenuItem("New Folder"))
					mDisplayNewFolderPopup = true;
				ImGui::EndPopup();
			}

			float cursorPos = ImGui::GetCursorPos().x;
			float cursorWidth = 0.f;

			for (const auto& file : mVisibleFiles)
			{
				bool doubleClicked = false;
				ImGuiExtension::File(file.DisplayName, file.TexId, doubleClicked);
				if (doubleClicked)
				{
					SetContentBrowserPath(file.FilePath);
					break;
				}
				DragDropFile(file.FilePath);
				if (file.IsDirectory)
				{
					ImGuiExtension::DragDropSource(ImGuiExtension::PAYLOAD_TYPE_FOLDER_PATH, file.FilePath, [&]() {
						ImGui::Image(file.TexId, { 50.f, 50.f });
						ImGui::Text(file.DisplayName.c_str());
						});
				}
				else
				{
					ImGuiExtension::DragDropSource(ImGuiExtension::PAYLOAD_TYPE_FILE_PATH, file.FilePath, [&]() {
						ImGui::Image(file.TexId, { 50.f, 50.f });
						ImGui::TextWrapped(file.DisplayName.c_str());
						});
				}
				if (FilePopContent(file))
					break;

				cursorWidth += ImGuiExtension::FileWidth + ImGui::GetStyle().ItemSpacing.x;

				if (cursorWidth + ImGuiExtension::FileWidth > ImGui::GetContentRegionAvail().x)
				{
					cursorWidth = 0.f;
				}
				else
					ImGui::SameLine();
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

bool ContentBrowserPanel::FilePopContent(const DisplayFile& file)
{
	bool refresh = false;
	if (ImGui::BeginPopupContextItem(file.DisplayName.c_str()))
	{
		if (ImGui::MenuItem(ICON_FA_TRASH" Delete"))
		{
			ImGui::Text(file.DisplayName.c_str());
			ImGui::Separator();
			fs::remove(file.FilePath);
			SetContentBrowserPath(mContentBrowserPath);
			refresh = true;
			// TODO: remove from asset manager
		}
		ImGui::EndPopup();
	}

	return refresh;
}

void ContentBrowserPanel::DisplayPopups()
{
	static char nameBuffer[256] = { 0 };
	if (mDisplayNewFolderPopup)
	{
		mDisplayNewFolderPopup = false;
		memset(nameBuffer, 0, 256);
		ImGui::OpenPopup("New Folder");
	}

	ImGui::SetNextWindowSize({350.f, 0.f});
	if (ImGui::BeginPopupModal("New Folder", nullptr, ImGuiWindowFlags_NoResize))
	{
		ImGui::Text("Name");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::InputText("##NewFolderName", nameBuffer, 256, ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiInputTextCallbackData* data) {
			static const char* invalidChars = "\\/:*?\"<>|"; // Windows forbidden characters
			if (strchr(invalidChars, (char)data->EventChar)) {
				return 1;
			}
			return 0;
			});

		ImGuiExtension::PushRedButtonStyle();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGuiExtension::PopRedButtonStyle();

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Create").x);

		ImGui::BeginDisabled(strlen(nameBuffer) == 0);
		if (ImGui::Button("Create"))
		{
			fs::create_directory(mContentBrowserPath / nameBuffer);
			SetContentBrowserPath(mContentBrowserPath);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::EndPopup();
	}

}
