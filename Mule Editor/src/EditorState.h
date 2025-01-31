#pragma once

#include "Mule.h"

enum SimulationState
{
	Editing,
	Simulation
};

struct EditorState
{
	Mule::Entity SelectedEntity;
	fs::path mProjectPath = "C:\\Development\\Mule Projects\\Test Project";
	fs::path mAssetsPath = "C:\\Development\\Mule Projects\\Test Project\\Assets";
	SimulationState SimulationState = SimulationState::Editing;
	Mule::EditorRenderSettings EditorRenderSettings;
};