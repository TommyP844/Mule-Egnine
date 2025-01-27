#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <glm/glm.hpp>
#include <iostream>

// Addition
static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
	return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

// Subtraction
static ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) {
	return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

// Scalar Multiplication
static ImVec2 operator*(const ImVec2& lhs, float scalar) {
	return ImVec2(lhs.x * scalar, lhs.y * scalar);
}

// Scalar Division
static ImVec2 operator/(const ImVec2& lhs, float scalar) {
	return ImVec2(lhs.x / scalar, lhs.y / scalar);
}

// Compound Assignment (+=, -=, etc.)
static ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

static ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

namespace ImguiUtil
{
	constexpr float FileWidth = 140.f;

	static bool Vec3(const std::string& name, glm::vec3& vector, const glm::vec3& resetVal = glm::vec3(0.f))
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

	static bool File(std::string name, ImTextureID texId, bool& doubleClicked)
	{
		ImVec2 size = { FileWidth, 210.f };

		ImVec2 padding = ImGui::GetStyle().ItemInnerSpacing;
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();

		bool ret = ImGui::InvisibleButton(name.c_str(), size);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			doubleClicked = true;
		bool hovered = ImGui::IsItemHovered();
		bool leftPressed = ImGui::IsMouseDown(0);

		ImGui::GetWindowDrawList()->AddRectFilled(cursorPos, cursorPos + size, IM_COL32(40, 40, 40, 255), 5.f, ImDrawFlags_RoundCornersBottom);
		ImGui::GetWindowDrawList()->AddRectFilled(cursorPos + ImVec2(0.f, size.x), cursorPos + size, IM_COL32(50, 50, 50, 255), 5.f, ImDrawFlags_RoundCornersBottom);
		ImGui::GetWindowDrawList()->AddImage(texId, cursorPos + padding, cursorPos + ImVec2(size.x, size.x) - padding);


		if (hovered)
		{
			if (leftPressed)
				ImGui::GetWindowDrawList()->AddRectFilled(cursorPos, cursorPos + size, IM_COL32(20, 20, 20, 150), 5.f, ImDrawFlags_RoundCornersBottom);
			else
				ImGui::GetWindowDrawList()->AddRectFilled(cursorPos, cursorPos + size, IM_COL32(20, 20, 20, 100), 5.f, ImDrawFlags_RoundCornersBottom);
		}


		ImFont* font = ImGui::GetFont();
		float fontSize = ImGui::GetFontSize();

		ImVec2 textPos = cursorPos + ImVec2(padding.x, size.x);
		ImVec2 textBegin = cursorPos + ImVec2(padding.x, size.x);
		ImVec2 textEnd = textBegin + ImVec2(size.x - padding.x * 2, ImGui::GetTextLineHeight() * 2.f);
		ImVec4 textClipRect = ImVec4(textBegin.x, textBegin.y, textEnd.x, textEnd.y);
		 
		ImGui::GetWindowDrawList()->AddText(
			font,
			fontSize, 
			textPos,
			IM_COL32(255, 255, 255, 255),
			name.c_str(),
			nullptr,
			size.x - padding.x * 2,
			&textClipRect);

		ImVec2 nextPos = ImGui::GetCursorScreenPos();

		return ret;
	}
}

