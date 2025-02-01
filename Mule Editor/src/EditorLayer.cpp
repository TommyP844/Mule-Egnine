#include "EditorLayer.h"

// Engine
#include "Mule.h"

// Editor
#include "Popups.h"

// Submodules
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <IconsFontAwesome6.h>

// Events
#include "Event/EditMaterialEvent.h"

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
	mMaterialEditorPanel.SetContext(mEditorState, context);

	mSceneHierarchyPanel.OnAttach();
	mSceneViewPanel.OnAttach();
	mComponentPanel.OnAttach();
	mContentBrowserPanel.OnAttach();
	mAssetManagerPanel.OnAttach();
	mMaterialEditorPanel.OnAttach();

	// mAssetManagerPanel.Close();

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

}

void EditorLayer::OnUpdate(float dt)
{
	SPDLOG_INFO("Layer OnUpdate: {}", GetName());
}

void EditorLayer::OnUIRender(float dt)
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
			ImGui::MenuItem("Material Editor", "", mMaterialEditorPanel.OpenPtr());
			ImGui::MenuItem("Scene Hierarchy", "", mSceneHierarchyPanel.OpenPtr());
			ImGui::MenuItem("Scene View", "", mSceneViewPanel.OpenPtr());
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	for (auto& event : mEditorState->GetEvents())
	{
		switch (event->GetEventType())
		{
		case EditorEventType::EditMaterial:
		{
			Ref<EditMaterialEvent> editMaterialEvent = event;
			mMaterialEditorPanel.Open();
			mMaterialEditorPanel.SetMaterial(editMaterialEvent->GetMaterialHandle());
		}
			break;
		}

		mComponentPanel.OnEvent(event);
		mSceneHierarchyPanel.OnEvent(event);
		mSceneViewPanel.OnEvent(event);
		mContentBrowserPanel.OnEvent(event);
		mAssetManagerPanel.OnEvent(event);
		mMaterialEditorPanel.OnEvent(event);
	}

	mEditorState->ClearEvents();

	mComponentPanel.OnUIRender(dt);
	mSceneHierarchyPanel.OnUIRender(dt);
	mSceneViewPanel.OnUIRender(dt);
	mContentBrowserPanel.OnUIRender(dt);
	mAssetManagerPanel.OnUIRender(dt);
	mMaterialEditorPanel.OnUIRender(dt);

	NewItemPopup(mNewScenePopup, "Scene", ".scene", mEditorState->mAssetsPath, [&](const fs::path& filepath) {
		Ref<Mule::Scene> scene = MakeRef<Mule::Scene>();
		scene->SetFilePath(filepath);
		mEngineContext->GetAssetManager()->InsertAsset(scene);
		mEngineContext->SetScene(scene);
		});
}

void EditorLayer::OnRender(float dt)
{
	auto scene = mEngineContext->GetScene();
	auto sceneRenderer = mEngineContext->GetSceneRenderer();
	if (scene)
	{
		switch (mEditorState->SimulationState)
		{
		case SimulationState::Editing:
		{
			mEditorState->EditorRenderSettings.SelectedEntities = { mEditorState->SelectedEntity };
			mEditorState->EditorRenderSettings.WaitSemaphores = { mEngineContext->GetGraphicsContext()->GetImageAcquiredGPUFence() };
			mEditorState->EditorRenderSettings.Scene = scene;
			sceneRenderer->OnEditorRender(mEditorState->EditorRenderSettings);
		}
			break;
		case SimulationState::Simulation:
			sceneRenderer->OnRender(scene, { mEngineContext->GetGraphicsContext()->GetImageAcquiredGPUFence() });
			break;
		default:
			break;
		}
	}
}

void EditorLayer::OnDetach()
{
	SPDLOG_INFO("Layer OnDetach: {}", GetName());
}
