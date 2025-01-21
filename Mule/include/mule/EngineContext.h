#pragma once

#include "Ref.h"
#include "Asset/AssetManager.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Context/ImGuiContext.h"
#include "Graphics/SceneRenderer.h"
#include "Application/Window.h"

namespace Mule
{
	struct EngineContextDescription
	{
		std::string WindowName;
		GraphicsContextDescription GraphicsDescription;
	};

	class EngineContext
	{
	public:
		EngineContext(EngineContextDescription& description);
		~EngineContext();

		Ref<GraphicsContext> GetGraphicsContext() const { return mGraphicsContext; }
		Ref<AssetManager> GetAssetManager() const { return mAssetManager; }
		Ref<SceneRenderer> GetSceneRenderer() const { return mSceneRenderer; }
		Ref<ImGuiContext> GetImGuiContext() const { return mImguiContext; }
		Ref<Window> GetWindow() const { return mWindow; }

		void SetScene(Ref<Scene> scene) { mScene = scene; }
		Ref<Scene> GetScene() const { return mScene; }

	private:
		Ref<Window> mWindow;
		Ref<GraphicsContext> mGraphicsContext;
		Ref<AssetManager> mAssetManager;
		Ref<SceneRenderer> mSceneRenderer;
		Ref<ImGuiContext> mImguiContext;
		Ref<Scene> mScene;
	};
}