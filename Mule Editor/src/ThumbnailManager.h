#pragma once

#include "EditorContext.h"

#include "Mule.h"

#include <map>
#include <set>
#include <atomic>

class ThumbnailManager
{
public:
	ThumbnailManager(WeakRef<Mule::EngineContext> context, WeakRef<EditorContext> editorContext);
	~ThumbnailManager();

	WeakRef<Mule::Texture2D> GetThumbnail(Mule::AssetHandle handle);
private:
	WeakRef<Mule::EngineContext> mEngineContext;
	WeakRef<EditorContext> mEditorContext;
	WeakRef<Mule::Texture2D> mLoadingImage;
	std::set<Mule::AssetHandle> mAssetsToRender;
	std::thread mRenderThread;
	std::mutex mMutex;
	std::atomic_bool mRunning;
	Mule::Camera mCamera;
	Ref<Mule::Scene> mScene;
	Ref<Mule::SceneRenderer> mSceneRenderer;
	Ref<Mule::Mesh> mSphereMesh;

	std::map<Mule::AssetHandle, Ref<Mule::Texture2D>> mThumbnails;

	void Renderthread();

	Ref<Mule::Texture2D> RenderModel(WeakRef<Mule::Model> model);
	Ref<Mule::Texture2D> RenderMaterial(WeakRef<Mule::Material> material);
	Ref<Mule::Texture2D> RenderScene(WeakRef<Mule::Scene> scene);
	void ScaleModel(Mule::Entity e, float scale);
};

