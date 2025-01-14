#pragma once

#include "Rendering/GraphicsContext.h"
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
		void SetGraphicsContext(Ref<GraphicsContext> graphicsContext);

		WeakRef<AssetManager> GetAssetManager() const { return mAssetManager; }
		Ref<Scene> GetActiveScene() const { return mActiveScene; }
		Ref<GraphicsContext> GetGraphicsContext() const { return mGraphicsContext; }
	private:
		Ref<Scene> mActiveScene;
		Ref<AssetManager> mAssetManager;
		Ref<GraphicsContext> mGraphicsContext;
	};
}