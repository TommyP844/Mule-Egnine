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
#include "Event/ViewtextureEvent.h"

EditorLayer::EditorLayer(Ref<Mule::EngineContext> context)
	:
	ILayer(context, "Editor Layer")
{
	mEditorState = MakeRef<EditorContext>("C:\\Development\\Mule Projects\\Test Project", context);

	
	std::string projectName = mEditorState->GetProjectName();
	const fs::path projectDLL = mEditorState->GetProjectPath() / "bin/Debug/net8.0" / (projectName + ".dll");
	mEngineContext->GetScriptContext()->LoadUserDLL(projectDLL);
	
	mSceneHierarchyPanel.SetContext(mEditorState, context);
	mSceneViewPanel.SetContext(mEditorState, context);
	mComponentPanel.SetContext(mEditorState, context);
	mContentBrowserPanel.SetContext(mEditorState, context);
	mAssetManagerPanel.SetContext(mEditorState, context);
	mMaterialEditorPanel.SetContext(mEditorState, context);
	mTextureViewerPanel.SetContext(mEditorState, context);
	mSceneRendererSettingsPanel.SetContext(mEditorState, context);
	mPrimitiveObjectPanel.SetContext(mEditorState, context);
	mPerformancePanel.SetContext(mEditorState, context);

	mSceneHierarchyPanel.OnAttach();
	mSceneViewPanel.OnAttach();
	mComponentPanel.OnAttach();
	mContentBrowserPanel.OnAttach();
	mAssetManagerPanel.OnAttach();
	mMaterialEditorPanel.OnAttach();
	mTextureViewerPanel.OnAttach();
	mSceneRendererSettingsPanel.OnAttach();
	mPrimitiveObjectPanel.OnAttach();
	mPerformancePanel.OnAttach();

	mAssetManagerPanel.Close();
	mMaterialEditorPanel.Close();
	mTextureViewerPanel.Close();

	ImGui::GetIO().Fonts->AddFontFromFileTTF("../Assets/Fonts/Roboto/Roboto-black.ttf", 18.f);
	ImFontConfig fontConfig;
	fontConfig.MergeMode = true;
	fontConfig.PixelSnapH = true;
	ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImGui::GetIO().Fonts->AddFontFromFileTTF("../Assets/Fonts/Font Awesome/fa-solid-900.ttf", 18.f, &fontConfig, &icon_ranges[0]);
	ImGui::GetIO().Fonts->Build();
}

EditorLayer::~EditorLayer()
{
	SPDLOG_TRACE("Shutting down editor layer");
}

void EditorLayer::OnEvent(Ref<Mule::Event> event)
{
	switch (event->Type)
	{
	case Mule::EventType::DropFileEvent:
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
		std::vector<std::future<void>> futures;
		for (auto dir : fs::recursive_directory_iterator(mEditorState->GetAssetsPath()))
		{
			if (dir.is_directory()) continue;

			std::string extension = dir.path().extension().string();
			fs::path filePath = dir.path();

			const std::set<std::string> modelExtensions = { ".gltf" };
			if (modelExtensions.contains(extension))
			{
				futures.push_back(std::async(std::launch::async, [=]() {
					mEngineContext->LoadAsset<Mule::Model>(filePath);
					}));
			}
			else if (extension == ".cs")
			{
				futures.push_back(std::async(std::launch::async, [=]() {
					mEngineContext->LoadAsset<Mule::ScriptClass>(filePath);
					}));
			}
		}

		for (const auto& f : futures)
			f.wait();
		futures.clear();

		for (auto dir : fs::recursive_directory_iterator(mEditorState->GetAssetsPath()))
		{
			if (dir.is_directory()) continue;

			std::string extension = dir.path().extension().string();
			fs::path filePath = dir.path();

			const std::set<std::string> imageExtensions = { ".jpg", ".jpeg", ".png", ".tga", ".bmp" };

			if (imageExtensions.contains(extension))
			{
				auto asset = mEngineContext->GetAssetByFilepath(dir.path());
				if (!asset)
				{
					futures.push_back(std::async(std::launch::async, [=]() {
						mEngineContext->LoadAsset<Mule::Texture2D>(filePath);
						}));
				}
			}
			else if (extension == ".hdr")
			{
				futures.push_back(std::async(std::launch::async, [=]() {
					mEngineContext->LoadAsset<Mule::EnvironmentMap>(filePath);
					}));
			}
			else if (extension == ".mat")
			{
				futures.push_back(std::async(std::launch::async, [=]() {
					mEngineContext->LoadAsset<Mule::Material>(filePath);
					}));
			}
			else if (extension == ".scene")
				mEngineContext->LoadAsset<Mule::Scene>(filePath);
		} 
		});

	mAssetLoaderThread.wait();
}

void EditorLayer::OnUpdate(float dt)
{
	auto scriptContext = mEditorState->GetScriptEditorContext();
	bool reloadDLL = scriptContext->DoesProjectDLLNeedReload();

	if (reloadDLL)
	{
		mEngineContext->GetScriptContext()->ReloadDLL();

		scriptContext->ClearProjectDLLNeedsReload();
	}

	switch (mEditorState->GetSimulationState())
	{
	case SimulationState::Editing: break;
	case SimulationState::Paused: break;
	case SimulationState::Simulation:
		mEngineContext->GetScene()->OnUpdate(dt);
		break;
	}
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
			if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Save", "Ctrl + S")) 
			{
				auto scene = mEngineContext->GetScene();
				if (scene)
				{
					mEngineContext->SaveAssetText<Mule::Scene>(scene->Handle());
					scene->ClearModified();
				}
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Scene"))
			{
				mNewScenePopup = true;
			}
			if (ImGui::MenuItem("Material"))
			{
				mNewMaterialPopup = true;
			}
			if (ImGui::MenuItem("Script"))
			{
				mNewScriptPopup = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Panels"))
		{
			ImGui::MenuItem("Asset Manager", "", mAssetManagerPanel.OpenPtr());
			ImGui::MenuItem("Content Browser", "", mContentBrowserPanel.OpenPtr());
			ImGui::MenuItem("Components", "", mComponentPanel.OpenPtr());
			ImGui::MenuItem("Material Editor", "", mMaterialEditorPanel.OpenPtr());
			ImGui::MenuItem("Performance", "", mPerformancePanel.OpenPtr());
			ImGui::MenuItem("Scene Hierarchy", "", mSceneHierarchyPanel.OpenPtr());
			ImGui::MenuItem("Scene View", "", mSceneViewPanel.OpenPtr());
			ImGui::MenuItem("Texture Viewer", "", mTextureViewerPanel.OpenPtr());
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
		case EditorEventType::ViewTexture:
		{
			Ref<ViewTextureEvent> viewTextureEvent = event;
			mTextureViewerPanel.Open();
			mTextureViewerPanel.SetTexture(viewTextureEvent->GetTextureHandle());
		}
		break;
		}

		mComponentPanel.OnEvent(event);
		mSceneHierarchyPanel.OnEvent(event);
		mSceneViewPanel.OnEvent(event);
		mContentBrowserPanel.OnEvent(event);
		mAssetManagerPanel.OnEvent(event);
		mMaterialEditorPanel.OnEvent(event);
		mTextureViewerPanel.OnEvent(event);
		mSceneRendererSettingsPanel.OnEvent(event);
		mPrimitiveObjectPanel.OnEvent(event);
		mPerformancePanel.OnEvent(event);
	}

	mEditorState->ClearEvents();

	//ImGui::ShowDemoWindow(&mShowDemoWindow);
	mComponentPanel.OnUIRender(dt);
	mSceneHierarchyPanel.OnUIRender(dt);
	mSceneViewPanel.OnUIRender(dt);
	mContentBrowserPanel.OnUIRender(dt);
	mAssetManagerPanel.OnUIRender(dt);
	mMaterialEditorPanel.OnUIRender(dt);
	mTextureViewerPanel.OnUIRender(dt);
	mSceneRendererSettingsPanel.OnUIRender(dt);
	mPrimitiveObjectPanel.OnUIRender(dt);
	mPerformancePanel.OnUIRender(dt);

	NewItemPopup(mNewScenePopup, "Scene", ".scene", mEditorState->GetAssetsPath(), [&](const fs::path& filepath) {
		Ref<Mule::Scene> scene = MakeRef<Mule::Scene>(mEngineContext);
		scene->SetFilePath(filepath);
		mEngineContext->InsertAsset(scene);
		mEngineContext->SetScene(scene);
		});

	NewItemPopup(mNewMaterialPopup, "Material", ".mat", mEditorState->GetAssetsPath(), [&](const fs::path& filepath) {
		Ref<Mule::Material> material = MakeRef<Mule::Material>();
		material->SetFilePath(filepath);
		mEngineContext->InsertAsset(material);
		mMaterialEditorPanel.Open();
		mMaterialEditorPanel.SetMaterial(material->Handle());
		});

	NewItemPopup(mNewScriptPopup, "Script", ".cs", mEditorState->GetAssetsPath(), [&](const fs::path& filepath) {
		mEditorState->GetScriptEditorContext()->CreateScriptFile(filepath);
		// TODO: notify Script context and reload dll
		});
}

void EditorLayer::OnRender(float dt)
{
	auto scene = mEngineContext->GetScene();
	auto sceneRenderer = mEngineContext->GetSceneRenderer();
	if (scene)
	{
		switch (mEditorState->GetSimulationState())
		{
		case SimulationState::Editing:
			sceneRenderer->OnEditorRender(scene, mEditorState->GetEditorCamera(), {});
			break;
		case SimulationState::Simulation:
			sceneRenderer->OnRender(scene, {});
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
