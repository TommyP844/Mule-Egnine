#pragma once

#include <mutex>
#include <thread>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

class ScriptEditorContext
{
public:
	ScriptEditorContext(const fs::path& projectPath);
	~ScriptEditorContext();

	void CreateScriptFile(const fs::path& filepath);

	bool DoesProjectDLLNeedReload() const { return mProjectDLLNeedsReload; }
	void ClearProjectDLLNeedsReload();

private:
	std::mutex mMutex;

	fs::path mProjectPath;
	
	bool mRunning;
	std::thread mFileMonitorThread;
	std::unordered_map<fs::path, uint64_t> mTrackedFiles;
	
	bool mProjectDLLNeedsReload;
	fs::path mProjectDLLPath;
	uint64_t mProjectDLLLastWriteTime;

	std::string GeneratePremakeFileText();

	void FileMonitorThread();
};