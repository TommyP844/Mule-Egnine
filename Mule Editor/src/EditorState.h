#pragma once

#include "Mule.h"

struct EditorState
{
	Mule::Entity SelectedEntity;
	fs::path mProjectPath = "C:\\Development\\Mule Projects\\Test Project";
	fs::path mAssetsPath = "C:\\Development\\Mule Projects\\Test Project\\Assets";
};