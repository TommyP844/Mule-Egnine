#include "EditorLayer.h"

#include "Mule.h"

#include <spdlog/spdlog.h>
#include <imgui.h>

#include <IconsFontAwesome6.h>


EditorLayer::EditorLayer(WeakRef<Mule::ApplicationData> appData)
	:
	ILayer(appData, "Editor Layer")
{
	mEditorState = Ref<EditorState>::Make();
	
	mSceneHierarchyPanel.SetContext(mEditorState, appData);
	mSceneViewPanel.SetContext(mEditorState, appData);
	mComponentPanel.SetContext(mEditorState, appData);
	mContentBrowserPanel.SetContext(mEditorState, appData);
}

void EditorLayer::OnAttach()
{
	SPDLOG_INFO("Layer attached: {}", GetName());

	ImGui::GetIO().Fonts->AddFontFromFileTTF("../Assets/Fonts/Roboto/Roboto-black.ttf", 18.f);
	ImFontConfig fontConfig;
	fontConfig.MergeMode = true;
	fontConfig.PixelSnapH = true;
	ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImGui::GetIO().Fonts->AddFontFromFileTTF("../Assets/Fonts/Font Awesome/fa-solid-900.ttf", 18.f, &fontConfig, &icon_ranges[0]);
	ImGui::GetIO().Fonts->Build();

	Ref<Mule::Scene> scene = mApplicationData->GetActiveScene();
	Mule::Entity a = scene->CreateEntity("A");
	Mule::Entity b = scene->CreateEntity("B");
	Mule::Entity c = scene->CreateEntity("C");

	a.AddChild(b);
	b.AddChild(c);
}

void EditorLayer::OnUpdate(float dt)
{
	SPDLOG_INFO("Layer OnUpdate: {}", GetName());	
}

void EditorLayer::OnUIRender()
{
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " New Project...", "Ctrl + Shift + N")) {}
			if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open Project", "Ctrl + Shift + O")) {}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Scene"))
			{
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Panels"))
		{
			ImGui::MenuItem("Content Browser", "", mContentBrowserPanel.OpenPtr());
			ImGui::MenuItem("Components", "", mComponentPanel.OpenPtr());
			ImGui::MenuItem("Scene Hierarchy", "", mSceneHierarchyPanel.OpenPtr());
			ImGui::MenuItem("Scene View", "", mSceneViewPanel.OpenPtr());
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	mComponentPanel.OnUIRender();
	mSceneHierarchyPanel.OnUIRender();
	mSceneViewPanel.OnUIRender();
	mContentBrowserPanel.OnUIRender();
}

void EditorLayer::OnDetach()
{
	SPDLOG_INFO("Layer OnDetach: {}", GetName());
}
