#include "EditorContext.h"

EditorContext::EditorContext(const fs::path& projectPath, WeakRef<Mule::EngineContext> engineContext)
	:
	mProjectPath(projectPath),
	mEngineContext(engineContext)
{
	mAssetsPath = mProjectPath / "Assets";
	if (!fs::exists(mAssetsPath))
		fs::create_directory(mAssetsPath);

	mProjectName = projectPath.filename().string();

	mScriptEditorContext = MakeRef<ScriptEditorContext>(mProjectPath);

	mAssetLoaderThread = std::async(std::launch::async, [&]() {
		std::vector<std::future<void>> futures;

		for (auto dir : fs::recursive_directory_iterator(GetAssetsPath()))
		{
			if (dir.is_directory()) continue;

			std::string extension = dir.path().extension().string();
			fs::path filePath = dir.path();

			const std::set<std::string> modelExtensions = { ".gltf", ".fbx", ".dae", ".obj" };
			const std::set<std::string> imageExtensions = { ".jpg", ".jpeg", ".png", ".tga", ".bmp" };

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
			else if (imageExtensions.contains(extension))
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
				futures.push_back(std::async(std::launch::async, [=]() {
				mEngineContext->LoadAsset<Mule::Scene>(filePath);
					}));
		}

		for (auto& future : futures)
		{
			future.wait();
		}

		});

}

void EditorContext::SetSelectedEntity(Mule::Entity e)
{
	if (mSelectedEntity)
	{
		mSelectedEntity.RemoveComponent<Mule::HighlightComponent>();
	}

	mSelectedEntity = e;
	auto sceneRenderer = mEngineContext->GetSceneRenderer();
	if (sceneRenderer)
		sceneRenderer->GetDebugOptions().SelectedEntity = e;

	if (mSelectedEntity)
	{
		if(!mSelectedEntity.HasComponent<Mule::HighlightComponent>())
			mSelectedEntity.AddComponent<Mule::HighlightComponent>();
	}
}

void EditorContext::SetSimulationState(SimulationState state)
{
	mSimulationState = state;
	switch (mSimulationState)
	{
	case SimulationState::Simulation:
		SetSelectedEntity(Mule::Entity());
		mSimulationScene = mEngineContext->GetScene()->Copy();
		mEngineContext->SetScene(mSimulationScene);
		mSimulationScene->OnPlayStart();
		break;
	case SimulationState::Paused: 
		// Noting to do yet
		break;
	case SimulationState::Editing:
		mSimulationScene->OnPlayStop();
		SetSelectedEntity(Mule::Entity());
		auto scene = mEngineContext->GetAsset<Mule::Scene>(mSimulationScene->Handle());
		mEngineContext->SetScene(scene);
		mSimulationScene = nullptr;
		break;
	}
}
