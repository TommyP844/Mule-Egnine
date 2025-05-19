#include "UIEditorPanel.h"

#include "ImGuiExtension.h"

UIEditorPanel::UIEditorPanel()
	:
	IPanel("UI Editor")
{
}

void UIEditorPanel::OnAttach()
{
	auto assetManager = mEngineContext->GetAssetManager();
	mBlackTexture = assetManager->Get<Mule::Texture2D>(MULE_BLACK_TEXTURE_HANDLE);

	mUIScene = MakeRef<Mule::UIScene>();
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
		float canvasWidth = width - elementPanelWidth - inspectorPanelWidth;

		if (ImGui::BeginChild("Elements", { elementPanelWidth, 0.f }, ImGuiChildFlags_Border))
		{
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

		ImGui::SameLine();

		if (ImGui::BeginChild("Inspector", { inspectorPanelWidth, 0.f }, ImGuiChildFlags_Border))
		{
			ImGui::SeparatorText("Inspector");
			DisplayInspectorPanel();
		}
		ImGui::EndChild();
	}

	ImGui::End();
}

void UIEditorPanel::OnEditorEvent(Ref<IEditorEvent> event)
{
}

void UIEditorPanel::OnEngineEvent(Ref<Mule::Event> event)
{
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

	if (region.x != mViewportSize.x || region.y != mViewportSize.y)
	{
		mViewportSize = region;
		mUIEditorCamera->GetRegistry()->Resize(region.x, region.y);
		mUIEditorCamera->SetAspectRatio(region.x / region.y);
	}

	Mule::UIRect windowRect(0, 0, region.x, region.y);
	auto scene = mEngineContext->GetScene();
	if (scene)
	{
		scene->RecordRuntimeDrawCommands();
		auto& commandList = scene->GetCommandList();
		mUIScene->Render(commandList, windowRect);
		Mule::Renderer::Get().Submit(*mUIEditorCamera, commandList);
		commandList.Flush();
		
		texId = mUIEditorCamera->GetColorOutput()->GetImGuiID();
	}
	else
	{
		Mule::CommandList commandList;
		mUIScene->Render(commandList, windowRect);
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
			mUIScene->AddUIElement(text);
		}
			break;
		case Mule::UIElementType::UIButton:
			break;
		case Mule::UIElementType::MAX_UI_ELEMENT_TYPE:
		default:
			assert("Invalid UIElementType");
			break;
		}
	}

	// TODO: draw grid lines
	for (auto element : mUIScene->GetUIElements())
	{
		Mule::UIRect windowRect(cursorPos.x, cursorPos.y, region.x, region.y);
		Mule::UIRect uiRect = element->GetTransform().CalculateRect(windowRect, Mule::UIRect(0.f, 0.f, 25.f, 25.f));
		ImVec2 pos = { uiRect.X, uiRect.Y };
		ImVec2 size = { uiRect.Width, uiRect.Height };
		
		// TODO: make name unique
		if (ImGuiExtension::DragBox(element->GetName(), pos, size))
		{
			float relativeLeft = (pos.x - windowRect.X);
			float relativeTop = (pos.y - windowRect.Y);
			float relativeWidth = size.x;
			float relativeHeight = size.y;

			Mule::UITransform& transform = element->GetTransform();

			transform.SetLeft(Mule::UIMeasurement(relativeLeft, Mule::UIUnitType::Pixels));
			transform.SetTop(Mule::UIMeasurement(relativeTop, Mule::UIUnitType::Pixels));
			transform.SetWidth(Mule::UIMeasurement(relativeWidth, Mule::UIUnitType::Pixels));
			transform.SetHeight(Mule::UIMeasurement(relativeHeight, Mule::UIUnitType::Pixels));
		}
	}
}

void UIEditorPanel::DisplayInspectorPanel()
{
}

void UIEditorPanel::DisplayElementSelection(Mule::UIElementType type)
{
	float width = ImGui::GetContentRegionAvail().x;
	std::string name = Mule::GetUIElementNameFromType(type);
	ImGui::PushItemWidth(width);
	ImGui::Selectable(name.c_str());
	ImGuiExtension::DragDropSource(ImGuiExtension::PAYLOAD_TYPE_UI_ELEMENT_TYPE, type, [name]() {
		ImGui::Text(name.c_str());
		});
}
