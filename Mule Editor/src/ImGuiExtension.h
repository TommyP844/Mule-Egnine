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
	constexpr const char* PAYLOAD_TYPE_ASSET = "Asset";
	constexpr const char* PAYLOAD_TYPE_UI_ELEMENT_TYPE = "UI_ELEMENT_TYPE";

	struct DragDropFile
	{
		char FilePath[260] = { 0 };
		bool IsDirectory = false;
		Mule::AssetHandle AssetHandle = Mule::AssetHandle::Null();
		Mule::AssetType AssetType = Mule::AssetType::None;
	};

	struct DragDropAsset
	{
		Mule::AssetHandle AssetHandle;
		Mule::AssetType AssetType;
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

	static void DrawDashedLine(ImVec2 p1, ImVec2 p2, float dashLength = 4.f, float gapLength = 2.f, ImU32 lineColor = IM_COL32(255, 255, 255, 255))
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 dir = p2 - p1;
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		ImVec2 norm = dir / len;
		float current = 0.f;
		while (current < len) {
			float start = current;
			float end = std::min(current + dashLength, len);
			drawList->AddLine(p1 + norm * start, p1 + norm * end, lineColor);
			current += dashLength + gapLength;
		}
	}

	static bool DragBox(const std::string& name, ImVec2& pos, ImVec2& size, const ImVec2& minSize = ImVec2(10.0f, 10.0f))
	{
		ImGui::PushID(name.c_str());

		ImGuiIO& io = ImGui::GetIO();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImU32 lineColor = IM_COL32(255, 255, 255, 255);
		const ImU32 cornerColor = IM_COL32(255, 20, 20, 255);
		const ImU32 midColor = IM_COL32(20, 20, 255, 255);
		ImVec2 min = pos;
		ImVec2 max = pos + size;

		const float dashLength = 4.0f;
		const float gapLength = 2.0f;
		ImVec2 handleSize = ImVec2(7.f, 7.f);
		ImVec2 halfHandle = handleSize * 0.5f;

		bool changed = false;

		// Helpers
		auto drawQuad = [&](ImVec2 center, ImU32 color) {
			drawList->AddRectFilled(center - halfHandle, center + halfHandle, color);
			};

		// Handle definitions
		struct Handle {
			ImVec2 center;
			ImVec2 dir; // (0, 1), (1, 0) or (1, 1) for corner/edge dragging
			ImU32 color;
			ImGuiMouseCursor Cursor;
		};

		std::vector<Handle> handles = {
			// Center (move)
			{ (min + max) * 0.5f, ImVec2(0, 0), 0, ImGuiMouseCursor_ResizeAll },

			// Corners
			{ min, ImVec2(-1, -1), cornerColor, ImGuiMouseCursor_ResizeNWSE },                    // Top-left
			{ ImVec2(max.x, min.y), ImVec2(1, -1), cornerColor, ImGuiMouseCursor_ResizeNESW },    // Top-right
			{ max, ImVec2(1, 1), cornerColor, ImGuiMouseCursor_ResizeNWSE },                      // Bottom-right
			{ ImVec2(min.x, max.y), ImVec2(-1, 1), cornerColor, ImGuiMouseCursor_ResizeNESW },    // Bottom-left

			// Edges (midpoints)
			{ (min + ImVec2(max.x, min.y)) * 0.5f, ImVec2(0, -1), midColor, ImGuiMouseCursor_ResizeNS }, // Top
			{ (ImVec2(max.x, min.y) + max) * 0.5f, ImVec2(1, 0), midColor, ImGuiMouseCursor_ResizeEW },  // Right
			{ (ImVec2(min.x, max.y) + max) * 0.5f, ImVec2(0, 1), midColor, ImGuiMouseCursor_ResizeNS },  // Bottom
			{ (min + ImVec2(min.x, max.y)) * 0.5f, ImVec2(-1, 0), midColor, ImGuiMouseCursor_ResizeEW }, // Left
		};

		static int activeHandle = -1;

		// Draw dashed border last
		DrawDashedLine(min, ImVec2(max.x, min.y));
		DrawDashedLine(ImVec2(max.x, min.y), max);
		DrawDashedLine(max, ImVec2(min.x, max.y));
		DrawDashedLine(ImVec2(min.x, max.y), min);

		struct LineArea {
			ImVec2 p1, p2;
		};

		const float edgeInset = 7.0f;
		const float midSize = 10.0f; // half-width around the midpoint to exclude

		ImVec2 midTop = ImVec2((min.x + max.x) * 0.5f, min.y);
		ImVec2 midRight = ImVec2(max.x, (min.y + max.y) * 0.5f);
		ImVec2 midBottom = ImVec2((min.x + max.x) * 0.5f, max.y);
		ImVec2 midLeft = ImVec2(min.x, (min.y + max.y) * 0.5f);

		std::vector<LineArea> edges = {
			// Top edge split
			{ ImVec2(min.x + edgeInset, min.y), ImVec2(midTop.x - midSize, min.y) },
			{ ImVec2(midTop.x + midSize, min.y), ImVec2(max.x - edgeInset, min.y) },

			// Right edge split
			{ ImVec2(max.x, min.y + edgeInset), ImVec2(max.x, midRight.y - midSize) },
			{ ImVec2(max.x, midRight.y + midSize), ImVec2(max.x, max.y - edgeInset) },

			// Bottom edge split
			{ ImVec2(max.x - edgeInset, max.y), ImVec2(midBottom.x + midSize, max.y) },
			{ ImVec2(midBottom.x - midSize, max.y), ImVec2(min.x + edgeInset, max.y) },

			// Left edge split
			{ ImVec2(min.x, max.y - edgeInset), ImVec2(min.x, midLeft.y + midSize) },
			{ ImVec2(min.x, midLeft.y - midSize), ImVec2(min.x, min.y + edgeInset) },
		};

		const float lineHitThickness = 6.0f;

		for (int i = 0; i < edges.size(); ++i) {
			ImVec2 p1 = edges[i].p1;
			ImVec2 p2 = edges[i].p2;

			// Expand along normal for hitbox
			ImVec2 dir = p2 - p1;
			float len = ImLengthSqr(dir) > 0.0f ? std::sqrt(dir.x * dir.x + dir.y * dir.y) : 1.0f;
			ImVec2 norm = ImVec2(-dir.y, dir.x) / len; // Perpendicular

			ImVec2 offset = norm * (lineHitThickness * 0.5f);
			ImVec2 a = p1 + offset;
			ImVec2 b = p2 + offset;
			ImVec2 c = p2 - offset;
			ImVec2 d = p1 - offset;

			// Create invisible button in a rectangle between a-b-c-d
			ImGui::SetCursorScreenPos(ImMin(a, c));
			ImVec2 size = ImVec2(ImAbs(c.x - a.x), ImAbs(c.y - a.y));
			size.x = glm::max(size.x, 2.f);
			size.y = glm::max(size.y, 2.f);
			ImGui::InvisibleButton(("line_drag_" + std::to_string(i)).c_str(), size);

			if (ImGui::IsItemHovered()) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
				if (ImGui::IsMouseClicked(0))
					activeHandle = 0; // Center (move entire box)
			}
		}

		for (int i = 1; i < handles.size(); ++i) {
			Handle& h = handles[i];

			// Determine rect for the handle
			ImVec2 topLeft = h.center - halfHandle;
			ImVec2 bottomRight = h.center + halfHandle;

			// Use ImGui style-based colors
			ImU32 fillColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
			ImU32 borderColor = ImGui::GetColorU32(ImGuiCol_Border);

			// Optional: distinguish corners from edges
			if (std::abs(h.dir.x) + std::abs(h.dir.y) == 2) { // corner
				fillColor = ImGui::GetColorU32(ImGuiCol_ButtonActive);
			}

			// Draw filled rectangle with border and optional rounding
			drawList->AddRectFilled(topLeft, bottomRight, fillColor, 2.0f);
			drawList->AddRect(topLeft, bottomRight, borderColor, 2.0f, 0, 1.5f);

			// Invisible button for interaction
			ImGui::SetCursorScreenPos(topLeft);
			ImGui::InvisibleButton(std::to_string(i).c_str(), handleSize);

			// Handle interaction
			if (ImGui::IsItemHovered()) {
				ImGui::SetMouseCursor(h.Cursor);
				if (ImGui::IsMouseClicked(0))
					activeHandle = i;
			}
		}

		if (activeHandle != -1 && ImGui::IsMouseDown(0)) {
			ImVec2 delta = io.MouseDelta;
			Handle& h = handles[activeHandle];

			// X-axis handling
			if (h.dir.x < 0) {
				pos.x += delta.x;
				size.x -= delta.x;
			}
			else if (h.dir.x > 0) {
				size.x += delta.x;
			}

			// Y-axis handling
			if (h.dir.y < 0) {
				pos.y += delta.y;
				size.y -= delta.y;
			}
			else if (h.dir.y > 0) {
				size.y += delta.y;
			}

			if (h.dir.x == 0 && h.dir.y == 0)
			{
				pos.x += delta.x;
				pos.y += delta.y;
			}

			// Clamp to min size
			if (size.x < minSize.x) size.x = minSize.x;
			if (size.y < minSize.y) size.y = minSize.y;

			changed = true;
		}
		else if (activeHandle != -1 && ImGui::IsMouseReleased(0)) {
			activeHandle = -1;
		}

		ImGui::PopID();

		return changed;
	}


#pragma endregion
}