#pragma once

#include "imgui.h"

#include <glm/glm.hpp>
#include <iostream>

namespace ImguiUtil
{
	bool Vec3(const std::string& name, glm::vec3& vector, const glm::vec3& resetVal = glm::vec3(0.f))
	{
		bool ret = false;

		float inputWidth = 70.f;

		ImGui::PushID(name.c_str());

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });

		// X
		{
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.9f, 0.f, 0.f, 1.f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.5f, 0.f, 0.f, 1.f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.5f, 0.25f, 0.25f, 1.f });

			if (ImGui::Button(("X##" + name).c_str()))
			{
				ret = true;
				vector.x = resetVal.x;
			}
			ImGui::SameLine();
			ImGui::PushItemWidth(inputWidth);
			ret |= ImGui::DragFloat(("##X" + name).c_str(), &vector.x, 1.f, 0.f, 0.f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}

		// Y
		{
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.f, 0.9f, 0.f, 1.f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.f, 0.5f, 0.f, 1.f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.25f, 0.5f, 0.25f, 1.f });

			if (ImGui::Button(("Y##" + name).c_str()))
			{
				ret = true;
				vector.y = resetVal.y;
			}
			ImGui::SameLine();
			ImGui::PushItemWidth(inputWidth);
			ret |= ImGui::DragFloat(("##Y" + name).c_str(), &vector.y, 1.f, 0.f, 0.f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}

		// Z
		{
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.f, 0.f, 0.9f, 1.f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.f, 0.f, 0.5f, 1.f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.25f, 0.25f, 0.5f, 1.f });

			if (ImGui::Button(("Z##" + name).c_str()))
			{
				ret = true;
				vector.z = resetVal.z;
			}
			ImGui::SameLine();
			ImGui::PushItemWidth(inputWidth);
			ret |= ImGui::DragFloat(("##Z" + name).c_str(), &vector.z, 1.f, 0.f, 0.f, "%.2f");
			ImGui::PopItemWidth();
		}

		ImGui::PopStyleColor(9);
		ImGui::PopStyleVar();

		ImGui::PopID();

		return ret;
	}
}