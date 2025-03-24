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

}

void EditorContext::SetSimulationState(SimulationState state)
{
	mSimulationState = state;
	switch (mSimulationState)
	{
	case SimulationState::Simulation:
		mSelectedEntity = Mule::Entity();
		mSimulationScene = mEngineContext->GetScene()->Copy();
		mEngineContext->SetScene(mSimulationScene);
		mSimulationScene->OnPlayStart();
		break;
	case SimulationState::Paused: 
		// Noting to do yet
		break;
	case SimulationState::Editing:
		mSimulationScene->OnPlayStop();
		mSelectedEntity = Mule::Entity();
		auto scene = mEngineContext->GetAsset<Mule::Scene>(mSimulationScene->Handle());
		mEngineContext->SetScene(scene);
		mSimulationScene = nullptr;
		break;
	}
}
