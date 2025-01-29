#include "EditorLayer.h"

// Engine
#include "Mule.h"

// Editor
#include "Popups.h"

// Submodules
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <IconsFontAwesome6.h>

EditorLayer::EditorLayer(Ref<Mule::EngineContext> context)
	:
	ILayer(context, "Editor Layer")
{
	mEditorState = MakeRef<EditorState>();
	
	mSceneHierarchyPanel.SetContext(mEditorState, context);
	mSceneViewPanel.SetContext(mEditorState, context);
	mComponentPanel.SetContext(mEditorState, context);
	mContentBrowserPanel.SetContext(mEditorState, context);
	mAssetManagerPanel.SetContext(mEditorState, context);

	mSceneHierarchyPanel.OnAttach();
	mSceneViewPanel.OnAttach();
	mComponentPanel.OnAttach();
	mContentBrowserPanel.OnAttach();
	mAssetManagerPanel.OnAttach();

	ImGui::GetIO().Fonts->AddFontFromFileTTF("../Assets/Fonts/Roboto/Roboto-black.ttf", 18.f);
	ImFontConfig fontConfig;
	fontConfig.MergeMode = true;
	fontConfig.PixelSnapH = true;
	ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImGui::GetIO().Fonts->AddFontFromFileTTF("../Assets/Fonts/Font Awesome/fa-solid-900.ttf", 18.f, &fontConfig, &icon_ranges[0]);
	ImGui::GetIO().Fonts->Build();

	// Test Code
	auto scene = MakeRef<Mule::Scene>();
	auto entity = scene->CreateEntity();
	entity.AddComponent<Mule::CameraComponent>();
	mEditorState->SelectedEntity = entity;
	mEngineContext->SetScene(scene);
}

EditorLayer::~EditorLayer()
{
	SPDLOG_TRACE("Shutting down editor layer");
}

void EditorLayer::OnEvent(Ref<Mule::Event> event)
{
	switch (event->Type)
	{
	case Mule::EventType::DropFile:
	{
		Ref<Mule::DropFileEvent> dropFileEvent = event;
		SPDLOG_INFO("Drop file event recieved: ");
		for (auto path : dropFileEvent->GetPaths())
		{
			SPDLOG_INFO("\t{}", path.string());
		}
	}

		break;
	}
}

void EditorLayer::OnAttach()
{
	SPDLOG_INFO("Layer attached: {}", GetName());

	mAssetLoaderThread = std::async(std::launch::async, [&]() {

		for (auto dir : fs::recursive_directory_iterator(mEditorState->mAssetsPath))
		{
			if (dir.is_directory()) continue;

			std::string extension = dir.path().extension().string();
			fs::path filePath = dir.path();

			const std::set<std::string> imageExtensions = { ".jpg", ".jpeg", ".png", ".tga", ".bmp" };
			const std::set<std::string> modelExtensions = { ".gltf" };

			if (imageExtensions.contains(extension))
			{
				mEngineContext->GetAssetManager()->LoadAsset<Mule::Texture2D>(filePath);
			}
			else if (modelExtensions.contains(extension))
			{
				mEngineContext->GetAssetManager()->LoadAsset<Mule::Model>(filePath);
			}
		}
		});

}

void EditorLayer::OnUpdate(float dt)
{
	SPDLOG_INFO("Layer OnUpdate: {}", GetName());
}

void EditorLayer::OnUIRender()
{
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpaceOverViewport();//ImGui::GetMainViewport(), dockspace_flags);

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
				mNewScenePopup = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Panels"))
		{
			ImGui::MenuItem("Asset Manager", "", mAssetManagerPanel.OpenPtr());
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
	mAssetManagerPanel.OnUIRender();

	NewItemPopup(mNewScenePopup, "Scene", ".scene", mEditorState->mAssetsPath, [&](const fs::path& filepath) {
		Ref<Mule::Scene> scene = MakeRef<Mule::Scene>();
		scene->SetFilePath(filepath);
		mEngineContext->GetAssetManager()->InsertAsset(scene);
		mEngineContext->SetScene(scene);
		});
}

void EditorLayer::OnDetach()
{
	SPDLOG_INFO("Layer OnDetach: {}", GetName());
}
