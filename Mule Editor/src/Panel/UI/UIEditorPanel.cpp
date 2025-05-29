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

	bool changed = false;

	ImGui::PushID("ModifyElement");

	ImVec2 cursorPos = ImGui::GetCursorPos();
	Mule::UIRect uiRect = mSelectedElement->GetFinalRect();

	ImVec2 min = ImVec2(uiRect.x, uiRect.y) + mFrameCursorPos;
	ImVec2 max = min + ImVec2(uiRect.width, uiRect.height);

	// Draw Rect
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->PushClipRect(mFrameCursorPos, mFrameCursorPos + mViewportSize);
	ImGuiExtension::DrawDashedLine(ImVec2(min.x, min.y), ImVec2(min.x, max.y)); // Left
	ImGuiExtension::DrawDashedLine(ImVec2(max.x, min.y), ImVec2(max.x, max.y)); // Right
	ImGuiExtension::DrawDashedLine(ImVec2(min.x, min.y), ImVec2(max.x, min.y)); // Top
	ImGuiExtension::DrawDashedLine(ImVec2(min.x, max.y), ImVec2(max.x, max.y)); // Bottom
	
	const float halfWidth = uiRect.width * 0.5f;
	const float halfHeight = uiRect.height * 0.5f;

	SnapCorner corners[4] = {
		//SnapCorner(ImGuiMouseCursor_ResizeNWSE, min, Mule::UIAnchorAxis::Top | Mule::UIAnchorAxis::Left, glm::vec2(1.f, 1.f)),							// Top Left
		SnapCorner(ImGuiMouseCursor_ResizeNWSE, ImVec2(min.x + halfWidth, min.y), Mule::UIAnchorAxis::Top, glm::vec2(0.f, 1.f)),						// Top
		//SnapCorner(ImGuiMouseCursor_ResizeNWSE, ImVec2(max.x, min.y), Mule::UIAnchorAxis::Top | Mule::UIAnchorAxis::Right, glm::vec2(1.f, 1.f)),		// Top Right
		SnapCorner(ImGuiMouseCursor_ResizeNWSE, ImVec2(min.x, min.y + halfHeight), Mule::UIAnchorAxis::Left, glm::vec2(1.f, 0.f)),						// Left
		SnapCorner(ImGuiMouseCursor_ResizeNWSE, ImVec2(max.x, min.y + halfHeight), Mule::UIAnchorAxis::Right, glm::vec2(1.f, 0.f)),						// Right
		//SnapCorner(ImGuiMouseCursor_ResizeNWSE, ImVec2(min.x, max.y), Mule::UIAnchorAxis::Bottom | Mule::UIAnchorAxis::Left, glm::vec2(1.f, 1.f)),		// Bottom Left
		SnapCorner(ImGuiMouseCursor_ResizeNWSE, ImVec2(min.x + halfWidth, max.y), Mule::UIAnchorAxis::Bottom, glm::vec2(0.f, 1.f)),						// Bottom
		//SnapCorner(ImGuiMouseCursor_ResizeNWSE, ImVec2(max.x, max.y), Mule::UIAnchorAxis::Bottom | Mule::UIAnchorAxis::Right, glm::vec2(1.f, 1.f)),		// Bottom Right
	};

	for (uint32_t i = 0; i < 4; i++)
	{
		SnapCorner& corner = corners[i];

		const ImVec2 snapDotHalfSize = ImVec2(5.f, 5.f);
		const ImVec2 snapDotMin = corner.Pos - snapDotHalfSize;
		const ImVec2 snapDotMax = corner.Pos + snapDotHalfSize;

		ImU32 boxColor = IM_COL32(51, 51, 51, 255);
		
		ImGui::SetCursorPos(corner.Pos - snapDotHalfSize);
		ImGui::InvisibleButton("##SnapDot", ImVec2(2.f * snapDotHalfSize.x, 2.f * snapDotHalfSize.y));
		if (ImGui::IsItemHovered())
		{
			boxColor = IM_COL32(31, 31, 31, 255);

			ImGui::SetMouseCursor(corner.cursor);
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				// Get Drag Delta
				const ImVec2 IV2dragDelta = ImGui::GetMouseDragDelta();

				// Multiply by corner axis enablement (glm::vec2, 1 for movement, 0 for no movement)
				const glm::vec2 dragDelta = glm::vec2(IV2dragDelta.x, IV2dragDelta.y) * corner.AxisMovement;

				Mule::UIMeasurement measurement = mSelectedElement->GetMeasurement(corner.axis);
				if (corner.AxisMovement.x == 1)
				{
					measurement.Value += dragDelta.x;
				}
				if (corner.AxisMovement.y == 1)
				{
					measurement.Value += dragDelta.y;
				}

				// append delta to element axis
				mSelectedElement->SetAxisMeasurement(corner.axis, measurement);
				boxColor = IM_COL32(10, 10, 10, 255);
			}
		}

		drawList->AddRectFilled(snapDotMin, snapDotMax, boxColor, 2.f, ImDrawFlags_RoundCornersAll);
		drawList->AddRect(snapDotMin - ImVec2(2.f, 2.f), snapDotMax + ImVec2(2.f, 2.f), IM_COL32(200, 200, 200, 255), 2.f, ImDrawFlags_RoundCornersAll);

		ImGui::SetCursorPos(cursorPos);
	}
	

	ImGui::PopID();

	return changed;
}
