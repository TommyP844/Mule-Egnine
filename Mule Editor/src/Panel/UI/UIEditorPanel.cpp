#include "UIEditorPanel.h"

#include "ImGuiExtension.h"
#include "Panel/UI/UIEditorDisplay.h"

#include "Event/EditUISceneEvent.h"

UIEditorPanel::UIEditorPanel()
	:
	IPanel("UI Editor")
{
}

void UIEditorPanel::OnAttach()
{
	auto assetManager = mEngineContext->GetAssetManager();
	mBlackTexture = assetManager->Get<Mule::Texture2D>(MULE_BLACK_TEXTURE_HANDLE);

	mUIEditorCamera = MakeRef<Mule::Camera>();
	auto registry = Mule::Renderer::Get().CreateResourceRegistry();
	mUIEditorCamera->SetResourceRegistry(registry);
	mSelectedElement = nullptr;

}

void UIEditorPanel::OnUIRender(float dt)
{
	if (!mIsOpen)
		return;

	if (ImGui::Begin(mName.c_str(), &mIsOpen))
	{
		if (!mUIScene)
		{
			ImGui::End();
			return;
		}
		float width = ImGui::GetContentRegionAvail().x;
		float elementPanelWidth = 200.f;
		float inspectorPanelWidth = 200.f;
		float canvasWidth = width - elementPanelWidth - 3.f * ImGui::GetStyle().FramePadding.x;

		if (ImGui::BeginChild("Elements", { elementPanelWidth, 0.f }, ImGuiChildFlags_Border))
		{
			auto assetManager = mEngineContext->GetAssetManager();
			ImGui::SeparatorText("UI Scene");

			ImGui::BeginDisabled(!mIsModified);
			if (ImGui::Button("Save"))
			{
				assetManager->Save<Mule::UIScene>(mUIScene->Handle());
				mIsModified = false;
			}
			ImGui::EndDisabled();

			auto theme = assetManager->Get<Mule::UITheme>(mUIScene->GetThemeHandle());
			std::string themeName = "Default";
			if (theme)
				themeName = theme->Name();

			ImGui::Text("Theme: %s", themeName.c_str());
			ImGuiExtension::DragDropFile ddf;
			if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
			{
				if (ddf.AssetType == Mule::AssetType::UITheme)
				{
					mUIScene->SetThemeHandle(ddf.AssetHandle);
				}
			}


			ImGui::SeparatorText("Elements");
			DisplayElementPanel();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("Canvas", { canvasWidth, 0.f }, ImGuiChildFlags_Border))
		{
			ImGui::SeparatorText("Canvas");
			DisplayCanvasPanel();
		}
		ImGui::EndChild();

	}
	ImGui::End();

	if (ImGui::Begin("UI Inspector"))
	{
		ImGui::SeparatorText("Inspector");
		DisplayInspectorPanel();
	}
	ImGui::End();
}

void UIEditorPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
	switch (event->GetEventType())
	{
	case EditorEventType::EditUIScene:
	{
		Ref<EditUISceneEvent> sceneEvent = event;
		auto assetManager = mEngineContext->GetAssetManager();
		mUIScene = assetManager->Get<Mule::UIScene>(sceneEvent->GetUISceneHandle());
		mSelectedElement = nullptr;
		mIsModified = false;
		mUIScene->SetAssetManager(assetManager);
	}
	break;
	}
}

void UIEditorPanel::OnEngineEvent(Ref<Mule::Event> event)
{
}

void UIEditorPanel::SetUIScene(WeakRef<Mule::UIScene> scene)
{
	mUIScene = scene;
	mSelectedElement = nullptr;
	mIsModified = false;
}

void UIEditorPanel::DisplayElementPanel()
{
	for (uint32_t i = 0; i < static_cast<uint32_t>(Mule::UIElementType::MAX_UI_ELEMENT_TYPE); i++)
	{
		Mule::UIElementType type = static_cast<Mule::UIElementType>(i);
		DisplayElementSelection(type);
	}
}

void UIEditorPanel::DisplayCanvasPanel()
{
	ImTextureID texId = mBlackTexture->GetImGuiID();

	ImVec2 region = ImGui::GetContentRegionAvail();
	mFrameCursorPos = ImGui::GetCursorScreenPos();

	Mule::UIRect windowRect(0, 0, region.x, region.y);
	
	auto assetManager = mEngineContext->GetAssetManager();
	
	mUIScene->Update(windowRect, assetManager);

	if (region.x != mViewportSize.x || region.y != mViewportSize.y)
	{
		mViewportSize = region;
		mUIEditorCamera->GetRegistry()->Resize(region.x, region.y);
		mUIEditorCamera->SetAspectRatio(region.x / region.y);
	}

	auto scene = mEngineContext->GetScene();
	if (scene)
	{
		scene->RecordRuntimeDrawCommands();
		auto& commandList = scene->GetCommandList();
		mUIScene->Render(commandList, windowRect, assetManager);
		Mule::Renderer::Get().Submit(*mUIEditorCamera, commandList);
		commandList.Flush();
		
		texId = mUIEditorCamera->GetColorOutput()->GetImGuiID();
	}
	else
	{
		Mule::CommandList commandList;
		mUIScene->Render(commandList, windowRect, assetManager);
		Mule::Renderer::Get().Submit(*mUIEditorCamera, commandList);

		texId = mUIEditorCamera->GetColorOutput()->GetImGuiID();
	}

	ImGui::Image(texId, region);

	Mule::UIElementType type;
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_UI_ELEMENT_TYPE, type))
	{
		ImVec2 mousePos = ImGui::GetMousePos() - mFrameCursorPos;
		switch (type)
		{
		case Mule::UIElementType::UIText:
		{
			Ref<Mule::UIText> text = MakeRef<Mule::UIText>("UI Text");
			text->SetLeft(mousePos.x, Mule::UIUnitType::Pixels);
			text->SetTop(mousePos.y, Mule::UIUnitType::Pixels);
			text->SetWidth(100.f, Mule::UIUnitType::Pixels);
			text->SetHeight(100.f, Mule::UIUnitType::Pixels);

			mUIScene->AddUIElement(text);
			mSelectedElement = text;
			return; // We need to call UIScene update before continuing to displaying the selected element
		}
			break;
		case Mule::UIElementType::UIButton:
		{
			Ref<Mule::UIButton> button = MakeRef<Mule::UIButton>("UI Text");
			button->SetLeft(mousePos.x, Mule::UIUnitType::Pixels);
			button->SetTop(mousePos.y, Mule::UIUnitType::Pixels);
			button->SetWidth(100.f, Mule::UIUnitType::Pixels);
			button->SetHeight(100.f, Mule::UIUnitType::Pixels);

			mUIScene->AddUIElement(button);
			mSelectedElement = button;
			return; // We need to call UIScene update before continuing to displaying the selected element
		}
			break;
		case Mule::UIElementType::MAX_UI_ELEMENT_TYPE:
		default:
			assert("Invalid UIElementType");
			break;
		}

		mIsModified = true;
	}

	ImVec2 mousePos = ImGui::GetMousePos() - mFrameCursorPos;
	bool dragging = ModifySelected();

	if (mousePos.x > 0.f && mousePos.x < region.x
		&& mousePos.y > 0.f && mousePos.y < region.y && !dragging)
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			auto element = mUIScene->HitTest(mousePos.x, mousePos.y);
			mSelectedElement = element;
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			auto element = mUIScene->HitTest(mousePos.x, mousePos.y);
			mSelectedElement = element;
			ImGui::OpenPopup("ElementPopup");
		}		
	}

	if (ImGui::BeginPopupContextItem("ElementPopup"))
	{
		if (ImGui::MenuItem("Delete"))
		{
			mUIScene->RemoveUIElement(mSelectedElement);
			mSelectedElement = nullptr;
			mIsModified = true;
		}
		ImGui::EndPopup();
	}

}

void UIEditorPanel::DisplayInspectorPanel()
{
	if (!mSelectedElement)
		return;

	std::string name = mSelectedElement->GetName();
	static char namebuffer[256] = { 0 };
	memcpy(namebuffer, name.data(), name.size());

	ImGui::Text("Name");
	ImGui::SameLine();
	if (ImGui::InputText("##name", namebuffer, 256))
	{
		mSelectedElement->SetName(namebuffer);
		mIsModified = true;
	}

	Mule::UITransform transform = mSelectedElement->GetTransform();

	ImGui::SeparatorText("Location");

	float parentWidth = mViewportSize.x;
	float parentHeight = mViewportSize.y;

	if (transform.Top)
	{
		if (DisplayUIMeasurement("Top", *transform.Top))
		{
			mSelectedElement->SetTop(transform.Top->Value, transform.Top->GetUnitType());
		}
	}
	if (transform.Left)
	{
		if (DisplayUIMeasurement("Left", *transform.Left))
		{
			mSelectedElement->SetLeft(transform.Left->Value, transform.Left->GetUnitType());
		}
	}
	if (transform.Width)
	{
		if (DisplayUIMeasurement("Width", *transform.Width))
		{
			mSelectedElement->SetWidth(transform.Width->Value, transform.Width->GetUnitType());
		}
	}
	if (transform.Height)
	{
		if (DisplayUIMeasurement("Height", *transform.Height))
		{
			mSelectedElement->SetHeight(transform.Height->Value, transform.Height->GetUnitType());
		}
	}

	//ImGui::SeparatorText("Style");
	std::string styleName = "(Null)";

	/*
	if (style)
		styleName = style->Name();

	ImGui::Text(styleName.c_str());
	ImGuiExtension::DragDropFile ddf;
	if (ImGuiExtension::DragDropTarget(ImGuiExtension::PAYLOAD_TYPE_FILE, ddf))
	{
		auto assetManager = mEngineContext->GetAssetManager();
		auto s = assetManager->Get<Mule::UIStyle>(ddf.AssetHandle);
		if (s)
		{
			mSelectedElement->SetStyle(s);
			mIsModified = true;
		}
	}
	*/

	auto elementType = mSelectedElement->GetType();
	std::string elementTypeName = Mule::ToString(elementType);
	ImGui::SeparatorText(elementTypeName.c_str());
	switch (mSelectedElement->GetType())
	{
	case Mule::UIElementType::UIText:
		mIsModified |= DisplayTextElementEditor(mSelectedElement);
		break;
	case Mule::UIElementType::UIButton:
		mIsModified |= DisplayButtonElementEditor(mSelectedElement);
		break;
	}
}

void UIEditorPanel::DisplayElementSelection(Mule::UIElementType type)
{
	float width = ImGui::GetContentRegionAvail().x;
	std::string name = Mule::ToString(type);
	ImGui::PushItemWidth(width);
	ImGui::Selectable(name.c_str()); 
	bool& mod = mIsModified;
	ImGuiExtension::DragDropSource(ImGuiExtension::PAYLOAD_TYPE_UI_ELEMENT_TYPE, type, [&]() {
		static std::string n = name; // We need to copy this or it will go out ous scope since using reference capture lamda
		ImGui::Text(n.c_str());
		mIsModified = true;
		});
}

void UIEditorPanel::SnapDraggingBox(const Mule::UIRect& draggedRect, const ImVec2& mousePos, float threshold)
{	
	float left = draggedRect.x;
	float right = draggedRect.x + draggedRect.width;
	float top = draggedRect.y;
	float bottom = draggedRect.y + draggedRect.height;
	float centerX = (left + right) * 0.5f;
	float centerY = (top + bottom) * 0.5f;

	for (auto element : mUIScene->GetUIElements())
	{
		if (element == mSelectedElement)
			continue;
	
		const Mule::UIRect& rect = element->GetFinalRect();
	
		float otherLeft = mousePos.x + rect.x;
		float otherRight = otherLeft + rect.width;
		float otherTop = mousePos.y + rect.y;
		float otherBottom = otherTop + rect.height;
		float otherCenterX = (otherLeft + otherRight) * 0.5f;
		float otherCenterY = (otherTop + otherBottom) * 0.5f;
	
		// Horizontal Snap (X axis)
		if (glm::abs(left - otherLeft) < threshold)
		{
			mSelectedElement->SetLeft(otherLeft, Mule::UIUnitType::Pixels);
		}
		else if (glm::abs(right - otherRight) < threshold)
		{
			mSelectedElement->SetRight(otherRight - rect.width, Mule::UIUnitType::Pixels);
		}
		//else if (glm::abs(centerX - otherCenterX) < threshold) result.SnapX = otherCenterX - boxSize.x * 0.5f;
	
		// Vertical Snap (Y axis)
		if (glm::abs(top - otherTop) < threshold)
		{
			mSelectedElement->SetTop(otherTop, Mule::UIUnitType::Pixels);
		}
		else if (glm::abs(bottom - otherBottom) < threshold)
		{
			mSelectedElement->SetBottom(otherBottom - rect.height, Mule::UIUnitType::Pixels);
		}
		//else if (glm::abs(centerY - otherCenterY) < threshold) result.SnapY = otherCenterY - boxSize.y * 0.5f;
	}
}

bool UIEditorPanel::ModifySelected()
{
	if (!mSelectedElement)
		return false;

	ImGui::PushID("ModifyElement");

	ImVec2 cursorPos = ImGui::GetCursorPos();
	Mule::UIRect uiRect = mSelectedElement->GetFinalRect();
	ImVec2 pos = ImVec2(uiRect.x, uiRect.y) + mFrameCursorPos;
	ImVec2 size = { uiRect.width, uiRect.height };

	const ImVec2 minSize = ImVec2(10.f, 10.f);

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
	ImGuiExtension::DrawDashedLine(min, ImVec2(max.x, min.y));
	ImGuiExtension::DrawDashedLine(ImVec2(max.x, min.y), max);
	ImGuiExtension::DrawDashedLine(max, ImVec2(min.x, max.y));
	ImGuiExtension::DrawDashedLine(ImVec2(min.x, max.y), min);

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

	bool leftSnapped = false;
	bool rightSnapped = false;
	bool topSnapped = false;
	bool bottomSnapped = false;
	bool verticalCenterSnapped = false;
	bool horizontalCenterSnapped = false;

	if (activeHandle != -1 && ImGui::IsMouseDown(0)) {
		ImVec2 delta = io.MouseDelta;
		Handle& h = handles[activeHandle];

		// Draw Snap Lines
		{
			ImVec2 mousePos = ImGui::GetMousePos() - mFrameCursorPos;
			// draw other bod lines
			for (auto element : mUIScene->GetUIElements())
			{
				if (element == mSelectedElement)
					continue;

				const Mule::UIRect& rect = element->GetFinalRect();
				const float displayLineThreshold = 15.f;
				const float snapDist = 7.f;

				float mouseLeftDist = glm::abs(mousePos.x - rect.x);
				float mouseRightDist = glm::abs(mousePos.x - rect.x - rect.width);
				float mouseTopDist = glm::abs(mousePos.y - rect.y);
				float mouseBottomDist = glm::abs(mousePos.y - rect.y - rect.height);
				float mouseHorizontalCenterDist = glm::abs(mousePos.x - rect.x - rect.width * 0.5f);
				float mouseVerticalCenterDist = glm::abs(mousePos.y - rect.y - rect.height * 0.5f);
  
				if (mouseLeftDist < displayLineThreshold && !mSelectedElement->IsAnchoredToElementAxis(element->GetHandle(), Mule::UIAnchorAxis::Left))
				{
					ImGuiExtension::DrawDashedLine({ mFrameCursorPos.x + rect.x, mFrameCursorPos.y }, { mFrameCursorPos.x + rect.x, mFrameCursorPos.y + mViewportSize.y }, 3.f, 3.f, IM_COL32(200, 20, 10, 255));
					if (mouseLeftDist <= snapDist)
					{
						if (h.dir.x < 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Left, Mule::UIAnchorAxis::Left);
							leftSnapped = true;
						}
						else if (h.dir.x > 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Left, Mule::UIAnchorAxis::Right);
							leftSnapped = true;
						}
					}
				}

				if (mouseRightDist < displayLineThreshold && !mSelectedElement->IsAnchoredToElementAxis(element->GetHandle(), Mule::UIAnchorAxis::Right))
				{
					ImGuiExtension::DrawDashedLine({ mFrameCursorPos.x + rect.x + rect.width, mFrameCursorPos.y }, { mFrameCursorPos.x + rect.x + rect.width, mFrameCursorPos.y + mViewportSize.y }, 3.f, 3.f, IM_COL32(200, 20, 10, 255));
					if (mouseRightDist <= snapDist)
					{
						if (h.dir.x < 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Right, Mule::UIAnchorAxis::Left);
							rightSnapped = true;
						}
						else if (h.dir.x > 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Right, Mule::UIAnchorAxis::Right);
							rightSnapped = true;
						}
					}
				}

				if (mouseTopDist < displayLineThreshold && !mSelectedElement->IsAnchoredToElementAxis(element->GetHandle(), Mule::UIAnchorAxis::Top))
				{
					ImGuiExtension::DrawDashedLine({ mFrameCursorPos.x, mFrameCursorPos.y + rect.y }, { mFrameCursorPos.x + mViewportSize.x, mFrameCursorPos.y + rect.y }, 3.f, 3.f, IM_COL32(200, 20, 10, 255));
					if (mouseTopDist <= snapDist)
					{
						if (h.dir.y < 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Top, Mule::UIAnchorAxis::Top);
							topSnapped = true;
						}
						else if (h.dir.y > 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Top, Mule::UIAnchorAxis::Bottom);
							topSnapped = true;
						}
					}
				}

				if (mouseBottomDist < displayLineThreshold && !mSelectedElement->IsAnchoredToElementAxis(element->GetHandle(), Mule::UIAnchorAxis::Bottom))
				{
					ImGuiExtension::DrawDashedLine({ mFrameCursorPos.x, mFrameCursorPos.y + rect.y + rect.height }, { mFrameCursorPos.x + mViewportSize.x, mFrameCursorPos.y + rect.y + rect.height }, 3.f, 3.f, IM_COL32(200, 20, 10, 255));
					if (mouseBottomDist <= snapDist)
					{
						if (h.dir.y < 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Bottom, Mule::UIAnchorAxis::Top);
							bottomSnapped = true;
						}
						else if (h.dir.y > 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::Bottom, Mule::UIAnchorAxis::Bottom);
							bottomSnapped = true;
						}
					}
				}

				// Vertical Center
				if (mouseVerticalCenterDist < displayLineThreshold && !mSelectedElement->IsAnchoredToElementAxis(element->GetHandle(), Mule::UIAnchorAxis::CenterVertical))
				{
					float yOffset = rect.y + rect.height * 0.5f;
					ImGuiExtension::DrawDashedLine({ mFrameCursorPos.x, mFrameCursorPos.y + yOffset }, { mFrameCursorPos.x + mViewportSize.x, mFrameCursorPos.y + yOffset }, 3.f, 3.f, IM_COL32(200, 20, 10, 255));
					if (mouseVerticalCenterDist <= snapDist)
					{
						if (h.dir.y < 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::CenterVertical, Mule::UIAnchorAxis::Top);
							verticalCenterSnapped = true;
						}
						else if (h.dir.y > 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::CenterVertical, Mule::UIAnchorAxis::Bottom);
							verticalCenterSnapped = true;
						}
						else if (h.dir.y == 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::CenterVertical, Mule::UIAnchorAxis::CenterVertical);
							verticalCenterSnapped = true;
						}
					}
				}

				// Horizontal Center
				if (mouseHorizontalCenterDist < displayLineThreshold && !mSelectedElement->IsAnchoredToElementAxis(element->GetHandle(), Mule::UIAnchorAxis::CenterVertical))
				{
					float xOffset = rect.x + rect.width * 0.5f;
					ImGuiExtension::DrawDashedLine({ mFrameCursorPos.x + xOffset, mFrameCursorPos.y }, { mFrameCursorPos.x + xOffset, mFrameCursorPos.y + mViewportSize.y }, 3.f, 3.f, IM_COL32(200, 20, 10, 255));
					if (mouseHorizontalCenterDist <= snapDist)
					{
						if (h.dir.x < 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::CenterHorizontal, Mule::UIAnchorAxis::Left);
							horizontalCenterSnapped = true;
						}
						else if (h.dir.x > 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::CenterHorizontal, Mule::UIAnchorAxis::Right);
							horizontalCenterSnapped = true;
						}
						else if (h.dir.x == 0)
						{
							mSelectedElement->AddAnchor(element->GetHandle(), Mule::UIAnchorAxis::CenterHorizontal, Mule::UIAnchorAxis::CenterHorizontal);
							horizontalCenterSnapped = true;
						}
					}
				}
			}
		}
		

		// X-axis handling
		if (h.dir.x < 0) {
			if (!leftSnapped && !rightSnapped)
			{
				pos.x += delta.x;
				//if(!horizontalCenterSnapped)
				//	mSelectedElement->RemoveAnchor(Mule::UIAnchorAxis::Left);
			}
			
			size.x -= delta.x;
		}
		else if (h.dir.x > 0) {
			if (!leftSnapped && !rightSnapped)
			{
				size.x += delta.x;
				//if(!horizontalCenterSnapped)
				//	mSelectedElement->RemoveAnchor(Mule::UIAnchorAxis::Right);
			}
		}

		// Y-axis handling
		if (h.dir.y < 0) 
		{
			if (!topSnapped && !bottomSnapped)
			{
				pos.y += delta.y;
				if(!verticalCenterSnapped)
					mSelectedElement->RemoveAnchor(Mule::UIAnchorAxis::Top);
			}
			size.y -= delta.y;
		}
		else if (h.dir.y > 0) 
		{
			if (!topSnapped && !bottomSnapped)
			{
				size.y += delta.y;
				if (!verticalCenterSnapped)
					mSelectedElement->RemoveAnchor(Mule::UIAnchorAxis::Bottom);
			}
		}

		if (h.dir.x == 0 && h.dir.y == 0)
		{
			if (!horizontalCenterSnapped)
			{
				//mSelectedElement->RemoveAnchor(Mule::UIAnchorAxis::Left);
				//mSelectedElement->RemoveAnchor(Mule::UIAnchorAxis::Right);
				pos.x += delta.x;
			}

			if (!verticalCenterSnapped)
			{
				//mSelectedElement->RemoveAnchor(Mule::UIAnchorAxis::Top);
				//mSelectedElement->RemoveAnchor(Mule::UIAnchorAxis::Bottom);
				pos.y += delta.y;
			}

			//if(!verticalCenterSnapped && !horizontalCenterSnapped)
			//	mSelectedElement->RemoveAllAnchors();
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

	if (changed)
	{
		float relativeLeft = pos.x - mFrameCursorPos.x;
		float relativeTop = pos.y - mFrameCursorPos.y;
		float relativeWidth = size.x;
		float relativeHeight = size.y;

		if(!leftSnapped)
			mSelectedElement->SetLeft(relativeLeft, Mule::UIUnitType::Pixels);

		if(!topSnapped)
			mSelectedElement->SetTop(relativeTop, Mule::UIUnitType::Pixels);

		if(!horizontalCenterSnapped)
			mSelectedElement->SetWidth(relativeWidth, Mule::UIUnitType::Pixels);

		if (!verticalCenterSnapped)
			mSelectedElement->SetHeight(relativeHeight, Mule::UIUnitType::Pixels);
	}

	mIsModified = true;

	return changed;
}
