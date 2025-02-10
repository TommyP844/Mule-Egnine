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
	SetContentBrowserPath(mEditorContext->mAssetsPath);
	mThumbnailManager = MakeRef<ThumbnailManager>(mEngineContext, mEditorContext);
	mExcludeExtensions = {
		".yml",
		".json",
		".md",
		".bin"
	};
}

void ContentBrowserPanel::OnUIRender(float dt)
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
	auto assetManager = mEngineContext->GetAssetManager();

	mContentBrowserPath = path;
	mVisibleFiles.clear();
	for (const auto& dir : fs::directory_iterator(mContentBrowserPath))
	{
		if (mExcludeExtensions.contains(dir.path().extension().string()))
			continue;
		DisplayFile file;
		file.IsDirectory = dir.is_directory();
		file.FilePath = dir.path();
		file.DisplayName = dir.path().filename().string();
		file.TexId = dir.is_directory() ? mFolderTexture->GetImGuiID() : mFileTexture->GetImGuiID();
		if (!dir.is_directory())
		{
			auto asset = assetManager->GetAssetByFilepath(dir.path());
			if (asset)
			{
				file.Handle = asset->Handle();
				file.AssetType = asset->GetType();
				if (file.AssetType == Mule::AssetType::Texture)
				{
					Ref<Mule::Texture2D> texture = asset;
					file.TexId = texture->GetImGuiID();
				}
				else
				{
					file.TexId = mThumbnailManager->GetThumbnail(asset->Handle())->GetImGuiID();
				}
			}

		}
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

void ContentBrowserPanel::ContentAssetDirBrowser(float width)
{
	if (ImGui::BeginChild("##Directories", { width, 0.f }, true))
	{
		float width = ImGui::GetContentRegionAvail().x;
		for (auto dir : fs::directory_iterator(mEditorContext->mAssetsPath))
		{
			if (!dir.is_directory())
				continue;
			if (ImGui::Button(dir.path().filename().string().c_str(), { width, 0.f }))
			{
				ClearSearchBuffer();
				SetContentBrowserPath(dir.path());
			}
			ImGuiExtension::DragDropFile ddf;
			if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
			{
				CopyDragDropFile(ddf, dir.path());
				SetContentBrowserPath(mContentBrowserPath);
			}
		}
	}
	ImGui::EndChild();
}

void ContentBrowserPanel::ContentFileBrowser(float width)
{
	if (ImGui::BeginChild("##Files", { width, 0.f }, true))
	{
		ImGui::BeginDisabled(mContentBrowserPath == mEditorContext->mAssetsPath);
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

		fs::path relativePath = fs::relative(mContentBrowserPath, mEditorContext->mAssetsPath);
		relativePath = (relativePath == ".") ? "Assets" : "Assets" / relativePath;
		fs::path curPath = mEditorContext->mAssetsPath.parent_path();
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
				ImGuiExtension::DragDropFile ddf;
				if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
				{
					CopyDragDropFile(ddf, curPath);
					SetContentBrowserPath(mContentBrowserPath);
				}

				ImGui::SameLine();
				ImGui::Text("/");
				ImGui::SameLine();
			}
			else
			{
				ImGui::Text((*iter).string().c_str());
			}
		}

		float width = ImGui::GetContentRegionAvail().x;
		float iconWidth = ImGui::CalcTextSize(ICON_FA_FILTER).x;
		ImGui::SameLine(width - iconWidth);
		if (ImGui::Button(ICON_FA_FILTER))
		{
			mDisplayFilterPopup = true;
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
				if (doubleClicked && file.IsDirectory)
				{
					SetContentBrowserPath(file.FilePath);
					break;
				}
				if (file.IsDirectory)
				{
					ImGuiExtension::DragDropFile ddf;
					if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
					{
						CopyDragDropFile(ddf, file.FilePath);
						SetContentBrowserPath(mContentBrowserPath);
						break;
					}
				}

				ImGuiExtension::DragDropFile ddf;

				ddf.AssetHandle = file.Handle;
				ddf.AssetType = file.AssetType;
				ddf.IsDirectory = file.IsDirectory;
				std::string fileStr = file.FilePath.string();
				memcpy(ddf.FilePath, fileStr.data(), fileStr.size());

				ImGuiExtension::DragDropSource(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf, [&]() {
					ImGui::Image(file.TexId, {});
					ImGui::Text(file.DisplayName.c_str());
					});
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

void ContentBrowserPanel::CopyDragDropFile(const ImGuiExtension::DragDropFile& file, const fs::path& newDir)
{
	fs::path oldPath = file.FilePath;
	fs::path name = oldPath.filename();
	fs::path newPath = newDir / name;
	fs::rename(file.FilePath, newPath);

	if (file.AssetHandle != Mule::NullAssetHandle)
	{
		auto asset = mEngineContext->GetAssetManager()->GetAsset<Mule::IAsset>(file.AssetHandle);
		asset->SetFilePath(newPath);
		asset = mEngineContext->GetAssetManager()->GetAsset<Mule::IAsset>(file.AssetHandle);
		SPDLOG_INFO("Path rename: {}, {}", asset->Handle(), asset->FilePath().string());
	}
}

void ContentBrowserPanel::DisplayPopups()
{
	static char nameBuffer[256] = { 0 };
	if (mDisplayNewFolderPopup)
	{
		mDisplayNewFolderPopup = false;
		memset(nameBuffer, 0, 256);
		ImGui::OpenPopup("New Folder");
		ImGui::SetNextWindowSize({ 350.f, 0.f }, ImGuiCond_Always);
	}
	if (mDisplayFilterPopup)
	{
		mDisplayFilterPopup = false;
		ImGui::OpenPopup("Filter");
		ImGui::SetNextWindowSize({350.f, 0.f}, ImGuiCond_Always);
	}


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

	if (ImGui::BeginPopupModal("Filter", nullptr))
	{
		static char buffer[16] = { 0 };
		if (ImGui::InputText("##FilterName", buffer, 16, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string ext = buffer;
			mExcludeExtensions.insert(ext);
			SetContentBrowserPath(mContentBrowserPath);
		}
		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			std::string ext = buffer;
			mExcludeExtensions.insert(ext);
			SetContentBrowserPath(mContentBrowserPath);
		}
		if (ImGui::BeginChild("##Extensions", {ImGui::GetContentRegionAvail().x, 200.f}, ImGuiChildFlags_Borders))
		{
			float xWidth = ImGui::CalcTextSize(ICON_FA_X).x;
			for (auto it = mExcludeExtensions.begin(); it != mExcludeExtensions.end(); it++)
			{
				ImGui::Text((*it).c_str());
				ImGui::SameLine(ImGui::GetContentRegionAvail().x - xWidth);
				ImGuiExtension::PushRedButtonStyle();
				ImGui::PushID((*it).c_str());
				bool b = false;
				if (ImGui::Button(ICON_FA_X))
				{
					b = true;
					mExcludeExtensions.erase(*it);
				}
				ImGui::PopID();
				ImGuiExtension::PopRedButtonStyle();
				if (b)
					break;
			}
		}
		ImGui::EndChild();
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

}
