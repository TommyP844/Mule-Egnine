#pragma once

#include "imgui.h"

#define TINYFD_NOLIB

#include <Windows.h>
#include "tinyfiledialogs.h"


static void CreateScenePopup(bool& show, const fs::path& defaultPath)
{
	static char pathBuffer[256] = { 0 };
	static char name[256] = { 0 };
	if (show)
	{
		ImGui::OpenPopup("New Scene");
		show = false;
		memset(pathBuffer, 0, 256);
		memcpy(pathBuffer, defaultPath.string().c_str(), defaultPath.string().size());

		static const char* defaultName = "Scene.scene";
		memset(name, 0, 256);
		memcpy(name, defaultName, strlen(defaultName));
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
			ImGui::InputText("##NewScenePath", pathBuffer, 256);
			
			ImGui::TableNextColumn();
			if (ImGui::Button("Browse"))
			{
				const char* selected = tinyfd_selectFolderDialog("Select Folder", pathBuffer);
				if (selected)
				{
					memset(pathBuffer, 0, 256);
					memcpy(pathBuffer, selected, strlen(selected));
				}
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::Text("Name");

			ImGui::TableNextColumn();
			ImGui::PushItemWidth(350.f);
			ImGui::InputText("##NewSceneName", name, 256);

			ImGui::TableNextColumn();

			
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Create").x);

		if (ImGui::Button("Create"))
		{
			// add .file extension if missing

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}