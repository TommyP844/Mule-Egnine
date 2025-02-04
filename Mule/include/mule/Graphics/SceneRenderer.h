#pragma once

// Engine
#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Execution/Semaphore.h"
#include "Graphics/Execution/Fence.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Camera.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/DescriptorSet.h"
#include "ECS/Scene.h"
#include "Asset/AssetManager.h"

#include "GuidArray.h"

// STD
#include <array>

namespace Mule
{
	struct EditorRenderSettings
	{
		Ref<Scene> Scene = nullptr;
		std::vector<WeakRef<Semaphore>> WaitSemaphores = {};
		Camera EditorCamera;
		std::vector<Entity> SelectedEntities;
	};

	class SceneRenderer
	{
	public:
		SceneRenderer(Ref<GraphicsContext> context, Ref<AssetManager> assetManager);
		~SceneRenderer();

		void OnRender(Ref<Scene> scene, std::vector<WeakRef<Semaphore>> waitSemaphore = {});
		void OnEditorRender(const EditorRenderSettings& settings);
		Ref<Semaphore> GetCurrentFrameRenderFinishedSemaphore() const { return mFrameData[mFrameIndex].RenderingFinishedSemaphore; }
		Ref<FrameBuffer> GetCurrentFrameBuffer() const { return mFrameData[mFrameIndex].Framebuffer; }
		void Resize(uint32_t width, uint32_t height);

	private:
		Ref<GraphicsContext> mGraphicsContext;
		Ref<AssetManager> mAssetManager;
		bool mIsValid;

		Ref<DescriptorSetLayout> mGeometryStageLayout;
		Ref<DescriptorSetLayout> mEnvironmentMapDescriptorSetLayout;

		struct GPUMaterial
		{
			glm::vec4 AlbedoColor;	
			glm::vec2 TextureScale;
			float MetalnessFactor;
			float RoughnessFactor;
			float AOFactor;
			uint32_t AlbedoIndex;
			uint32_t NormalIndex;
			uint32_t MetalnessIndex;
			uint32_t RoughnessIndex;
			uint32_t AOIndex;
			uint32_t EmissiveIndex;
		};

		struct FrameData
		{
			Ref<FrameBuffer> Framebuffer;
			Ref<Semaphore> RenderingFinishedSemaphore;
			Ref<Fence> RenderingFinishedFence;
			Ref<CommandPool> CommandPool;
			Ref<CommandBuffer> SolidGeometryPassCmdBuffer;
			Ref<UniformBuffer> CameraBuffer;
			Ref<UniformBuffer> MaterialBuffer;
			Ref<DescriptorSet> DescriptorSet;
			bool ResizeRequired = false;
			uint32_t ResizeWidth = 0;
			uint32_t ResizeHeight = 0;

			GuidArray<Ref<ITexture>> TextureArray;
			GuidArray<GPUMaterial> MaterialArray;

			// Environment Mapping
			Ref<Mule::DescriptorSet> EnvironmentMapDescriptorSet;
		};
		std::array<FrameData, 2> mFrameData;
		uint32_t mFrameIndex;

		Ref<RenderPass> mMainRenderPass;
		Ref<GraphicsShader> mDefaultGeometryShader;
		Ref<GraphicsShader> mEnvironmentMapShader;
		Ref<Mesh> mEnvironmentCube;


		struct CameraData
		{
			glm::mat4 View;
			glm::mat4 Proj;
		};

	};
}