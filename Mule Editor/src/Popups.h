#pragma once

#include "imgui.h"

// Submodules
#include "nfd.h"

// STd
#include <functional>

static void NewItemPopup(bool& show, const std::string& itemName, const std::string& ext, const fs::path& defaultPath, std::function<void(const fs::path&)> func)
{
	static char pathBuffer[256] = { 0 };
	static char name[256] = { 0 };
	if (show)
	{
		ImGui::OpenPopup(("New " + itemName).c_str());
		show = false;
		memset(pathBuffer, 0, 256);
		memcpy(pathBuffer, defaultPath.string().c_str(), defaultPath.string().size());

		memset(name, 0, 256);
	}

	if (ImGui::BeginPopupModal("New Scene", nullptr, ImGuiWindowFlags_NoResize))
	{
		if (ImGui::BeginTable("NewSceneTable", 3, ImGuiTableFlags_NoClip | ImGuiTableFlags_SizingFixedFit, {ImGui::GetContentRegionAvail().x, 0.f}))
		{
			ImGui::TableSetupColumn("Label");
			ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Action");

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			
			ImGui::Text("Filepath");

			ImGui::TableNextColumn();
			ImGui::PushItemWidth(350.f);
			ImGui::InputText("##NewItemPath", pathBuffer, 256);
			
			ImGui::TableNextColumn();
			if (ImGui::Button("Browse"))
			{
				nfdchar_t* outPath = NULL;
				nfdresult_t result = NFD_PickFolder(pathBuffer, &outPath);
				if (result == NFD_OKAY)
				{
					memset(pathBuffer, 0, 256);
					memcpy(pathBuffer, outPath, strlen(outPath));
				}
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::Text("Name");

			ImGui::TableNextColumn();
			ImGui::PushItemWidth(350.f);
			ImGui::InputText("##NewItemName", name, 256);

			ImGui::TableNextColumn();

			
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Create").x);

		if (ImGui::Button("Create"))
		{
			fs::path filepath = std::string(pathBuffer) + "\\" + name;
			if (filepath.extension() != ext)
			{
				filepath += ext;
			}

			func(filepath);

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}