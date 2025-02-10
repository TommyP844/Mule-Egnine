#pragma once

#include "Mule.h"
#include "Event/IEditorEvent.h"

#include <vector>

enum SimulationState
{
	Editing,
	Simulation
};

class EditorContext
{
public:
	Mule::Entity SelectedEntity;
	fs::path mProjectPath = "C:\\Development\\Mule Projects\\Test Project";
	fs::path mAssetsPath = "C:\\Development\\Mule Projects\\Test Project\\Assets";
	SimulationState SimulationState = SimulationState::Editing;
	Mule::EditorRenderSettings EditorRenderSettings;

	void PushEvent(Ref<IEditorEvent> event) { mEvents.push_back(event); }
	const std::vector<Ref<IEditorEvent>>& GetEvents() const { return mEvents; }
	void ClearEvents() { mEvents.clear(); }

private:
	std::vector<Ref<IEditorEvent>> mEvents;
};