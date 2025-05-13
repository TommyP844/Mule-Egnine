#include "EditorContext.h"

#include "Util.h"

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

	auto registry = Mule::Renderer::Get().CreateResourceRegistry();
	mEditorCamera.SetResourceRegistry(registry);

	auto jobSystem = mEngineContext->GetServiceManager()->Get<Mule::JobSystem>();
	auto assetManager = mEngineContext->GetAssetManager();

	for (auto dir : fs::recursive_directory_iterator(GetAssetsPath()))
	{
		if (dir.is_directory()) continue;

		std::string extension = dir.path().extension().string();
		fs::path filePath = dir.path();

		if (IsModelExtension(dir.path()))
		{
			jobSystem->PushJob([assetManager, filePath]() {
					assetManager->Load<Mule::Model>(filePath);
				});
		}
		else if (extension == ".cs")
		{
			jobSystem->PushJob([assetManager, filePath]() {
				assetManager->Load<Mule::ScriptClass>(filePath);
				});
		}
		else if (IsTextureExtension(dir.path()))
		{
			auto asset = assetManager->GetByFilepath(dir.path());
			if (!asset)
			{
				jobSystem->PushJob([assetManager, filePath]() {
					assetManager->Load<Mule::Texture2D>(filePath);
					});
			}
		}
		else if (extension == ".envmap")
		{
			jobSystem->PushJob([assetManager, filePath]() {
				assetManager->Load<Mule::EnvironmentMap>(filePath);
				});
		}
		else if (extension == ".mat")
		{
			jobSystem->PushJob([assetManager, filePath]() {
				assetManager->Load<Mule::Material>(filePath);
				});
		}
		else if (extension == ".scene")
		{
			jobSystem->PushJob([assetManager, filePath]() {
				assetManager->Load<Mule::Scene>(filePath);
				});
		}
	}
}

EditorContext::~EditorContext()
{
}

void EditorContext::SetSelectedEntity(Mule::Entity e)
{
	if (mSelectedEntity)
	{
		mSelectedEntity.RemoveComponent<Mule::HighlightComponent>();
	}

	mSelectedEntity = e;

	if (mSelectedEntity)
	{
		if(!mSelectedEntity.HasComponent<Mule::HighlightComponent>())
			mSelectedEntity.AddComponent<Mule::HighlightComponent>();
	}
}

void EditorContext::SetSimulationState(SimulationState state)
{
	auto assetManager = mEngineContext->GetAssetManager();
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
		auto scene = assetManager->Get<Mule::Scene>(mSimulationScene->Handle());
		mEngineContext->SetScene(scene);
		mSimulationScene = nullptr;
		break;
	}
}
