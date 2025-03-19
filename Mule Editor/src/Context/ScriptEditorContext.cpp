#include "ScriptEditorContext.h"

#include <spdlog/spdlog.h>

#include <fstream>


ScriptEditorContext::ScriptEditorContext(const fs::path& projectPath)
	:
	mProjectPath(projectPath),
	mRunning(true),
	mProjectDLLLastWriteTime(UINT64_MAX),
	mProjectDLLNeedsReload(false)
{
	auto premakePath = mProjectPath / "premake5.lua";
	if (!fs::exists(premakePath))
	{
		std::ofstream premakeFile(premakePath);
		if (!premakeFile)
		{
			SPDLOG_ERROR("Failed to create premake file");
			return;
		}

		premakeFile << GeneratePremakeFileText();

		premakeFile.close();
	}

	std::string filename = mProjectPath.filename().replace_extension().string();
	mProjectDLLPath = mProjectPath / "bin/Debug/net8.0" / (filename + ".dll");
	if (fs::exists(mProjectDLLPath))
	{
		mProjectDLLLastWriteTime = fs::last_write_time(mProjectDLLPath).time_since_epoch().count();
	}

	mFileMonitorThread = std::thread(&ScriptEditorContext::FileMonitorThread, this);
}

ScriptEditorContext::~ScriptEditorContext()
{
	mRunning = false;
	mFileMonitorThread.join();
}

void ScriptEditorContext::CreateScriptFile(const fs::path& filepath)
{
	if (fs::exists(filepath))
		return;

	std::ofstream scriptFile(filepath);
	if (!scriptFile)
	{
		SPDLOG_ERROR("Failed to create script: {}", filepath.string());
		return;
	}

	std::string filename = filepath.filename().replace_extension().string();
	filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());

	scriptFile << "Using Mule;";
	scriptFile << "";
	scriptFile << "class " << filename;
	scriptFile << "{";
	scriptFile << "};";

	scriptFile.close();
}

void ScriptEditorContext::ClearProjectDLLNeedsReload()
{
	std::lock_guard<std::mutex> lock(mMutex);
	mProjectDLLNeedsReload = false;
}

std::string ScriptEditorContext::GeneratePremakeFileText()
{
	std::string filename = mProjectPath.filename().replace_extension().string();

	std::string text;

	text += "workspace \"" + filename + "\"\n";
	text += "	configurations { \"Debug\", \"Release\" }\n";
	text += "	project \"" + filename + "\"\n";
	text += "		location \"\"\n";
	text += "		language \"C#\"\n";
	text += "		dotnetframework \"net8.0\"\n";
	text += "		kind \"SharedLib\"\n";
	text += "		clr \"Unsafe\"\n";
	text += "\n";
	text += "		files{\n";
	text += "			\"Assets/**.cs\"\n";
	text += "		}\n";

	return text;
}

void ScriptEditorContext::FileMonitorThread()
{
	while (mRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		const fs::path assetPath = mProjectPath / "Assets";

		if (!fs::exists(assetPath))
			continue;

		bool rebuild = false;


		for (auto dir : fs::recursive_directory_iterator(assetPath))
		{
			if (dir.is_directory())
				continue;

			std::string ext = dir.path().extension().string();
			fs::path curFilePath = dir.path();
			uint64_t curFileLastWriteTime = dir.last_write_time().time_since_epoch().count();

			if (ext == ".cs")
			{
				auto iter = mTrackedFiles.find(curFilePath);
				if (iter == mTrackedFiles.end())
				{
					mTrackedFiles[curFilePath] = curFileLastWriteTime;
					rebuild = true;
				}
				else
				{
					auto lastWriteTime = iter->second;
					if (lastWriteTime != curFileLastWriteTime)
					{
						rebuild = true;
						iter->second = curFileLastWriteTime;
					}
				}
			}
		}

		if (rebuild)
		{
			SPDLOG_INFO("Rebuilding C# Project");

			std::string command = "\"cd /d \"" + mProjectPath.string() + "\" && \"" + mProjectPath.string() + "/premake5.exe\" vs2022\"";
			system(command.c_str());

			command = "\"dotnet build \"" + (mProjectPath / mProjectPath.filename()).string() + ".csproj" + "\"\"";
			system(command.c_str());

			mProjectDLLNeedsReload = true;
		}

		std::lock_guard<std::mutex> lock(mMutex);
		if (fs::exists(mProjectDLLPath))
		{
			uint64_t lastWriteTime = fs::last_write_time(mProjectDLLPath).time_since_epoch().count();
			if (mProjectDLLLastWriteTime != lastWriteTime)
			{
				mProjectDLLLastWriteTime = lastWriteTime;
				mProjectDLLNeedsReload = true;
			}
		}
	}
}

