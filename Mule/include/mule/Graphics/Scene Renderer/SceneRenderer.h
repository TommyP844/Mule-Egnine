#pragma once

// Engine
#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Execution/Semaphore.h"
#include "Graphics/Execution/Fence.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Camera.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture/TextureCube.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Material.h"
#include "Graphics/Scene Renderer/SceneRenderPass/ISceneRenderPass.h"
#include "ECS/Scene.h"
#include "Asset/AssetManager.h"
#include "GPUObjects.h"
#include "GuidArray.h"

// Scene Render Passes
#include "SceneRenderPass/GeometryPass.h"
#include "SceneRenderPass/ShadowPass.h"

// STD
#include <vector>

namespace Mule
{
	struct SceneRendererSettings
	{
		bool EnableShadows = true;
		float Gamma = 2.2f;
		float Exposure = 1.f;

		// Temp
		bool ViewCascadedShadowMaps = false;
		uint32_t CascadeIndex = 0;
	};

	struct EditorRenderSettings
	{
		WeakRef<Scene> Scene = nullptr;
		std::vector<WeakRef<Semaphore>> WaitSemaphores = {};
		Camera EditorCamera;
		std::vector<Entity> SelectedEntities;
	};

	class SceneRenderer
	{
	public:
		SceneRenderer(Ref<GraphicsContext> context, Ref<AssetManager> assetManager);
		~SceneRenderer();

		void RefreshEngineObjects();

		void AddTexture(WeakRef<ITexture> texture);
		void RemoveTexture(AssetHandle textureHandle);

		void AddMaterial(WeakRef<Material> material);
		void RemoveMaterial(AssetHandle materialHandle);

		void OnRender(WeakRef<Scene> scene, std::vector<WeakRef<Semaphore>> waitSemaphore = {});
		void OnEditorRender(const EditorRenderSettings& settings);
		Ref<Semaphore> GetCurrentFrameRenderFinishedSemaphore() const { return mGeometryPass->GetSemaphore(); }
		Ref<FrameBuffer> GetCurrentFrameBuffer() const { return mGeometryPass->GetFB(); }
		void Resize(uint32_t width, uint32_t height);

		WeakRef<ShadowPass> GetShadowPass() const { return mShadowPass; }

		SceneRendererSettings& GetSettings() { return mSettings; }
	private:
		std::mutex mMutex;

		Ref<GraphicsContext> mGraphicsContext;
		Ref<AssetManager> mAssetManager;

		uint32_t mWhiteImageIndex;
		uint32_t mBlackImageIndex;
		uint32_t mBlackCubeMapImageIndex;

		GuidArray<WeakRef<ITexture>> mTextureArray;
		GuidArray<GPU::GPUMaterial> mMaterialArray;

		SceneRendererSettings mSettings;

		Ref<DescriptorSetLayout> mBindlessTextureDSL;

		// Render Passes
		Ref<GeometryPass> mGeometryPass;
		Ref<ShadowPass> mShadowPass;

		struct FrameData;
		std::vector<FrameData> mFrameData;
		uint32_t mFrameIndex;

		struct FrameData
		{			
			Ref<FrameBuffer> Framebuffer;
			Ref<Semaphore> RenderingFinishedSemaphore;
			Ref<Fence> RenderingFinishedFence;
			
			Ref<UniformBuffer> CameraBuffer;
			Ref<UniformBuffer> MaterialBuffer;
			Ref<UniformBuffer> LightBuffer;
			Ref<UniformBuffer> SceneSettingsBuffer;

			Ref<DescriptorSet> BindlessTextureDS;

			GPU::GPULightData LightData;
			
			bool ResizeRequired = false;
			uint32_t ResizeWidth = 0;
			uint32_t ResizeHeight = 0;

			std::vector<std::pair<WeakRef<ITexture>, uint32_t>> AddedTextures;
			std::vector<std::pair<GPU::GPUMaterial, uint32_t>> AddedMaterials;
		};

		void PrepDrawData(WeakRef<Scene> scene, const Camera& camera);
	};
}