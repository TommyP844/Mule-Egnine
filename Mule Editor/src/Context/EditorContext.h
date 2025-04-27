#pragma once

#include "Mule.h"
#include "Event/IEditorEvent.h"
#include "ScriptEditorContext.h"

#include <vector>

enum SimulationState
{
	Editing,
	Simulation,
	Paused
};

class EditorContext
{
public:
	EditorContext(const fs::path& projectPath, WeakRef<Mule::EngineContext> engineContext);
	~EditorContext();

	const fs::path& GetProjectPath() const { return mProjectPath; }
	const fs::path& GetAssetsPath() const { return mAssetsPath; }
	const std::string& GetProjectName() const { return mProjectName; }

	void SetSelectedEntity(Mule::Entity e);
	Mule::Entity GetSelectedEntity() const { return mSelectedEntity; }

	Mule::Camera& GetEditorCamera() { return mEditorCamera; }

	void SetSimulationState(SimulationState state);
	SimulationState GetSimulationState() const { return mSimulationState; }

	// Events
	void PushEvent(Ref<IEditorEvent> event) { mEvents.push_back(event); }
	const std::vector<Ref<IEditorEvent>>& GetEvents() const { return mEvents; }
	void ClearEvents() { mEvents.clear(); }

	// Scripting
	WeakRef<ScriptEditorContext> GetScriptEditorContext() const { return mScriptEditorContext; }

private:
	WeakRef<Mule::EngineContext> mEngineContext;
	std::vector<Ref<IEditorEvent>> mEvents;
	Ref<ScriptEditorContext> mScriptEditorContext;
	Ref<Mule::Scene> mSimulationScene;

	Mule::Entity mSelectedEntity;
	fs::path mProjectPath;
	fs::path mAssetsPath;	
	std::string mProjectName;

	SimulationState mSimulationState = SimulationState::Editing;
	Mule::Camera mEditorCamera;

	std::future<void> mAssetLoaderThread;
};