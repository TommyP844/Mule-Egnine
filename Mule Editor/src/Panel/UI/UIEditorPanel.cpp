#include "UIEditorPanel.h"

#include "ImGuiExtension.h"

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
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();

	Mule::UIRect windowRect(0, 0, region.x, region.y);
	mUIScene->Update(windowRect);

	if (region.x != mViewportSize.x || region.y != mViewportSize.y)
	{
		mViewportSize = region;
		mUIEditorCamera->GetRegistry()->Resize(region.x, region.y);
		mUIEditorCamera->SetAspectRatio(region.x / region.y);
	}

	auto assetManager = mEngineContext->GetAssetManager();
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
		ImVec2 mousePos = ImGui::GetMousePos() - cursorPos;
		switch (type)
		{
		case Mule::UIElementType::UIText:
		{
			Ref<Mule::UIText> text = MakeRef<Mule::UIText>("UI Text");
			text->GetTransform().SetLeft(Mule::UIMeasurement(mousePos.x, Mule::UIUnitType::Pixels));
			text->GetTransform().SetTop(Mule::UIMeasurement(mousePos.y, Mule::UIUnitType::Pixels));
			text->GetTransform().SetWidth(Mule::UIMeasurement(100.f, Mule::UIUnitType::Pixels));
			text->GetTransform().SetHeight(Mule::UIMeasurement(100.f, Mule::UIUnitType::Pixels));

			mUIScene->AddUIElement(text);
			mSelectedElement = text;
			return; // We need to call UIScene update before continuing to displaying the selected element
		}
			break;
		case Mule::UIElementType::UIButton:
			break;
		case Mule::UIElementType::MAX_UI_ELEMENT_TYPE:
		default:
			assert("Invalid UIElementType");
			break;
		}

		mIsModified = true;
	}

	if (mSelectedElement)
	{
		Mule::UIRect uiRect = mSelectedElement->GetScreenRect();
		ImVec2 pos = ImVec2(uiRect.X, uiRect.Y) + cursorPos;
		ImVec2 size = { uiRect.Width, uiRect.Height };

		if (ImGuiExtension::DragBox(mSelectedElement->GetName(), pos, size))
		{
			float relativeLeft = pos.x - cursorPos.x;
			float relativeTop = pos.y - cursorPos.y;
			float relativeWidth = size.x;
			float relativeHeight = size.y;

			Mule::UITransform& transform = mSelectedElement->GetTransform();

			transform.SetLeft(Mule::UIMeasurement(relativeLeft, Mule::UIUnitType::Pixels));
			transform.SetTop(Mule::UIMeasurement(relativeTop, Mule::UIUnitType::Pixels));
			transform.SetWidth(Mule::UIMeasurement(relativeWidth, Mule::UIUnitType::Pixels));
			transform.SetHeight(Mule::UIMeasurement(relativeHeight, Mule::UIUnitType::Pixels));

			mIsModified = true;
		}
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		ImVec2 mousePos = ImGui::GetMousePos() - cursorPos;
		auto element = mUIScene->HitTest(mousePos.x, mousePos.y);
		if (element)
		{
			mSelectedElement = element;
		}
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

	Mule::UITransform& transform = mSelectedElement->GetTransform();
	WeakRef<Mule::UIStyle> style = mSelectedElement->GetStyle();

	auto& left = transform.GetLeft();
	auto& top = transform.GetTop();
	auto& width = transform.GetWidth();
	auto& height = transform.GetHeight();

	ImGui::SeparatorText("Location");

	float parentWidth = mViewportSize.x;
	float parentHeight = mViewportSize.y;

	auto parent = mSelectedElement->GetParent();
	if (parent)
	{
		parentWidth = parent->GetScreenRect().Width;
		parentHeight = parent->GetScreenRect().Height;
	}

	if (top) DisplayUIMeasurement("Top", *top, parentHeight);
	if (left) DisplayUIMeasurement("Left", *left, parentWidth);
	if (width) DisplayUIMeasurement("Width", *width, parentWidth);
	if (height) DisplayUIMeasurement("Height", *height, parentHeight);

	ImGui::SeparatorText("Style");
	std::string styleName = "(Null)";

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

	switch (mSelectedElement->GetType())
	{
	case Mule::UIElementType::UIText:
	{
		WeakRef<Mule::UIText> textElem = mSelectedElement;
		static char buffer[1024] = { 0 };
		std::string text = textElem->GetText();
		memset(buffer, 0, 1024);
		memcpy(buffer, text.data(), text.size());
		ImGui::Text("Text");
		ImGui::SameLine();
		if (ImGui::InputTextMultiline("##Text", buffer, 1024, {250, 150}))
		{
			textElem->SetText(buffer);
			mIsModified = true;
		}		
	}
		break;
	}
}

void UIEditorPanel::DisplayElementSelection(Mule::UIElementType type)
{
	float width = ImGui::GetContentRegionAvail().x;
	std::string name = Mule::GetUIElementNameFromType(type);
	ImGui::PushItemWidth(width);
	ImGui::Selectable(name.c_str()); 
	bool& mod = mIsModified;
	ImGuiExtension::DragDropSource(ImGuiExtension::PAYLOAD_TYPE_UI_ELEMENT_TYPE, type, [&]() {
		static std::string n = name; // We need to copy this or it will go out ous scope since using reference capture lamda
		ImGui::Text(n.c_str());
		mIsModified = true;
		});
}

void UIEditorPanel::DisplayUIMeasurement(const char* label, Mule::UIMeasurement& measurement, float parentSize)
{
	ImGui::PushID(label);
	ImGui::Text(label);
	ImGui::SameLine(75.f);
	ImGui::PushItemWidth(100.f);
	if (ImGui::DragFloat("##MeasurementValue", &measurement.Value, 1.f, 0.f, 0.f, "%.1f"))
	{
		mIsModified = true;
	}
	ImGui::SameLine();

	const char* options[] = {
		"px",
		"%"
	};

	Mule::UIUnitType currentType = measurement.GetUnitType();
	const char* selectedOption = options[static_cast<uint32_t>(currentType)];

	ImGui::PushItemWidth(50.f);
	if (ImGui::BeginCombo("Combo", selectedOption))
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(Mule::UIUnitType::MAX_UNIT_TYPE); i++)
		{
			Mule::UIUnitType type = static_cast<Mule::UIUnitType>(i);
			bool selected = (type == currentType);

			if (ImGui::Selectable(options[i], selected))
			{
				measurement.SetUnitType(static_cast<Mule::UIUnitType>(i), parentSize);
				mIsModified = true;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
}
