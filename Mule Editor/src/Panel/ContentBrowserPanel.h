#pragma once

#include "IPanel.h"

#include "Mule.h"

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

	struct DisplayFile
	{
		ImTextureID TexId;
		bool IsDirectory;
		fs::path FilePath;
		std::string DisplayName;
	};

	std::vector<DisplayFile> mVisibleFiles;
	char mSearchBuffer[256] = { 0 };

	void SetContentBrowserPath(const fs::path& path, const std::string& filter = "");
	void DragDropFile(const fs::path& target);
	void ClearSearchBuffer();

	// Content
	void ContentAssetDirBrowser(float width);
	void ContentFileBrowser(float width);
	bool FilePopContent(const DisplayFile& file);

	// Popups
	void DisplayPopups();
	bool mDisplayNewFolderPopup = false;
};