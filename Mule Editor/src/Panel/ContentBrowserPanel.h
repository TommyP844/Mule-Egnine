#pragma once

#include "IPanel.h"
#include "ImGuiExtension.h"
#include "ThumbnailManager.h"

#include "Mule.h"

#include <set>

class ContentBrowserPanel : public IPanel
{
public:
	ContentBrowserPanel();
	~ContentBrowserPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender(float dt) override;
	virtual void OnEvent(Ref<IEditorEvent> event) override {}

private:
	Ref<Mule::Texture2D> mFolderTexture;
	Ref<Mule::Texture2D> mFileTexture;
	fs::path mContentBrowserPath;
	Ref<ThumbnailManager> mThumbnailManager;

	struct DisplayFile
	{
		ImTextureID TexId;
		bool IsDirectory;
		fs::path FilePath;
		std::string DisplayName;
		Mule::AssetType AssetType;
		Mule::AssetHandle Handle = Mule::NullAssetHandle;
	};

	std::vector<DisplayFile> mVisibleFiles;
	std::set<std::string> mExcludeExtensions;
	char mSearchBuffer[256] = { 0 };

	void SetContentBrowserPath(const fs::path& path, const std::string& filter = "");
	void ClearSearchBuffer();

	// Content
	void ContentAssetDirBrowser(float width);
	void ContentFileBrowser(float width);
	bool FilePopContent(const DisplayFile& file);
	void CopyDragDropFile(const ImGuiExtension::DragDropFile& file, const fs::path& newDir);

	// Popups
	void DisplayPopups();
	bool mDisplayNewFolderPopup = false;
	bool mDisplayFilterPopup = false;
};