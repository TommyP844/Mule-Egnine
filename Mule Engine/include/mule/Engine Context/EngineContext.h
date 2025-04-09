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

#pragma region Asset Mananger

		template<typename T>
		WeakRef<T> LoadAsset(const fs::path& filepath);

		template<typename T>
		WeakRef<T> GetAsset(AssetHandle handle);

		template<typename T>
		void InsertAsset(Ref<T> asset);

		template<typename T>
		void SaveAssetText(AssetHandle handle);

		void RemoveAsset(AssetHandle handle);
		std::vector<Ref<IAsset>> GetAssetsOfType(AssetType type) const;
		const std::unordered_map<AssetHandle, Ref<IAsset>>& GetAllAssets() const;
		Ref<IAsset> GetAssetByFilepath(const fs::path& path);

		void UpdateAssetHandle(AssetHandle oldHandle, AssetHandle newHandle);

#pragma endregion

		Ref<GraphicsContext> GetGraphicsContext() const { return mGraphicsContext; }
		Ref<SceneRenderer> GetSceneRenderer() const { return mSceneRenderer; }
		Ref<ImGuiContext> GetImGuiContext() const { return mImguiContext; }
		Ref<Window> GetWindow() const { return mWindow; }
		WeakRef<ScriptContext> GetScriptContext() const { return mScriptContext; }

		void SetScene(WeakRef<Scene> scene) { mScene = scene; }
		WeakRef<Scene> GetScene() const { return mScene; }

	private:
		fs::path mFilePath;
		Ref<Window> mWindow;
		Ref<GraphicsContext> mGraphicsContext;
		Ref<AssetManager> mAssetManager;
		Ref<SceneRenderer> mSceneRenderer;
		Ref<ImGuiContext> mImguiContext;
		Ref<ScriptContext> mScriptContext;
		WeakRef<Scene> mScene;

		std::future<void> mAssetLoadFuture;
	};
}

#include "EngineContext.inl"