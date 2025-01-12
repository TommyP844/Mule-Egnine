#pragma once

#include "ECS/Scene.h"
#include "Asset/AssetManager.h"
#include "Ref.h"

namespace Mule
{
	class ApplicationData
	{
	public: 
		ApplicationData();
		~ApplicationData();

		void Shutdown();

		void SetActiveScene(Ref<Scene> scene) { mActiveScene = scene; }

		WeakRef<AssetManager> GetAssetManager() const { return mAssetManager; }
		Ref<Scene> GetActiveScene() const { return mActiveScene; }
	private:
		Ref<Scene> mActiveScene;
		Ref<AssetManager> mAssetManager;
	};
}