#pragma once

/*
* Pre optimization:
*	Sponza Scene:
*		FPS: ~320
*		DT:   0.003s
*/

#include "Ref.h"
#include "Asset/AssetManager.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Context/ImGuiContext.h"
#include "Graphics/SceneRenderer.h"
#include "Application/Window.h"
#include "Services/ServiceManager.h"

namespace Mule
{
	struct EngineContextDescription
	{
		std::string WindowName;
		GraphicsContextDescription GraphicsDescription;
		fs::path ProjectPath;
	};

	class ScriptContext;

	class EngineContext
	{
	public:
		EngineContext(EngineContextDescription& description);
		~EngineContext();

		WeakRef<Window> GetWindow() const { return mWindow; }
		WeakRef<GraphicsContext> GetGraphicsContext() const;
		WeakRef<SceneRenderer> GetSceneRenderer() const;
		WeakRef<ImGuiContext> GetImGuiContext() const;
		WeakRef<ScriptContext> GetScriptContext() const;
		WeakRef<AssetManager> GetAssetManager() const;

		void SetScene(WeakRef<Scene> scene) { mScene = scene; }
		WeakRef<Scene> GetScene() const { return mScene; }

	private:
		fs::path mFilePath;
		Ref<Window> mWindow;
		WeakRef<Scene> mScene;

		Ref<ServiceManager> mServiceManager;

		std::future<void> mAssetLoadFuture;

		void LoadEngineAssets();
	};
}