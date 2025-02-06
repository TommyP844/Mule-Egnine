#pragma once

#include <imgui.h>
#include "imgui_internal.h"

#include <glm/glm.hpp>
#include <iostream>

// STD
#include <functional>
#include <string>
#include <filesystem>

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

namespace ImGuiExtension
{
#pragma region Drag Drop

	constexpr const char* PAYLOAD_TYPE_ENTITY = "Entity";
	constexpr const char* PAYLOAD_TYPE_EXTERNAL_FILE = "ExternalFilePath";
	constexpr const char* PAYLOAD_TYPE_FILE = "ExternalFilePath";

	struct DragDropFile
	{
		char FilePath[260] = { 0 };
		bool IsDirectory = false;
		Mule::AssetHandle AssetHandle = Mule::NullAssetHandle;
		Mule::AssetType AssetType = Mule::AssetType::None;
	};

	template<typename T>
	static void DragDropSource(const char* payloadType, const T& data, std::function<void()> displayFunc = nullptr)
	{
		if (ImGui::BeginDragDropSource())
		{
			if(displayFunc)
				displayFunc();
			ImGui::SetDragDropPayload(payloadType, &data, sizeof(T));
			ImGui::EndDragDropSource();
		}
	}

	template<>
	static void DragDropSource(const char* payloadType, const std::string& data, std::function<void()> displayFunc)
	{
		if (ImGui::BeginDragDropSource())
		{
			if (displayFunc)
				displayFunc();
			ImGui::SetDragDropPayload(payloadType, data.c_str(), data.size() + 1);
			ImGui::EndDragDropSource();
		}
	}

	template<>
	static void DragDropSource(const char* payloadType, const std::filesystem::path& data, std::function<void()> displayFunc)
	{
		if (ImGui::BeginDragDropSource())
		{
			if (displayFunc)
				displayFunc();
			std::string str = data.string();
			ImGui::SetDragDropPayload(payloadType, str.data(), str.size() + 1);
			ImGui::EndDragDropSource();
		}
	}

	template<typename T>
	static bool DragDropTarget(const char* payloadType, T& data)
	{
		bool recieved = false;
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType))
			{
				recieved = true;
				memcpy(&data, payload->Data, payload->DataSize);
			}
			ImGui::EndDragDropTarget();
		}
		return recieved;
	}

	static bool DragDropTarget(const char* payloadType, std::string& data)
	{
		bool recieved = false;
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType))
			{
				recieved = true;
				data = std::string((char*)payload->Data, payload->DataSize);
			}
			
			ImGui::EndDragDropTarget();
		}
		return recieved;
	}

	static bool DragDropTarget(const char* payloadType, std::filesystem::path& data)
	{
		bool recieved = false;
		if (ImGui::BeginDragDropTarget())
		{	
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType))
			{
				recieved = true;
				data = std::string((char*)payload->Data, payload->DataSize);
			}
		
			ImGui::EndDragDropTarget();
		}
		return recieved;
	}

#pragma endregion

#pragma region Extension

	constexpr float FileWidth = 140.f;

	static void PushRedButtonStyle()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.01f, 0.01f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.3f, 0.3f, 1.f));
	}

	static void PopRedButtonStyle()
	{
		ImGui::PopStyleColor(3);
	}

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
			ImGui::SameLine(0.f, 4.f);
			ImGui::PushItemWidth(inputWidth);
			ret |= ImGui::DragFloat(("##X" + name).c_str(), &vector.x, 1.f, 0.f, 0.f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine(0.f, 4.f);
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
			ImGui::SameLine(0.f, 4.f);
			ImGui::PushItemWidth(inputWidth);
			ret |= ImGui::DragFloat(("##Y" + name).c_str(), &vector.y, 1.f, 0.f, 0.f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine(0.f, 4.f);
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
			ImGui::SameLine(0.f, 4.f);
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

#pragma endregion
}