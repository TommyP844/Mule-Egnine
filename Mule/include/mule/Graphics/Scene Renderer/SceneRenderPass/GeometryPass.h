#pragma once

#include "ISceneRenderPass.h"

#include <WeakRef.h>

#include "ECS/Entity.h"

#include "Graphics/Camera.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/Buffer/UniformBuffer.h"
#include "Graphics/DescriptorSetLayout.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/GraphicsShader.h"
#include "Graphics/ComputeShader.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Execution/CommandPool.h"
#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Scene Renderer/GuidArray.h"
#include "Graphics/Scene Renderer/GPUObjects.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Asset/AssetManager.h"

#include "ECS/Scene.h"

namespace Mule
{
	struct GeometryPassFrameInfo
	{
		Ref<UniformBuffer> CameraBuffer;
		Ref<UniformBuffer> MaterialBuffer;
		Ref<UniformBuffer> LightBuffer;
		Ref<DescriptorSet> BindlessTextureDS;
	};

	struct GeometryPassInitInfo
	{
		WeakRef<AssetManager> AssetManager;
		GuidArray<GPU::GPUMaterial>& MaterialArray;
		GuidArray<WeakRef<ITexture>>& TextureArray;
		
		std::vector<GeometryPassFrameInfo> FrameInfo;
	};

	struct GeometryPassRenderInfo
	{
		WeakRef<Scene> Scene;
		Entity SelectedEntity;
		Camera Camera;
		std::vector<WeakRef<Semaphore>> WaitSemaphores;
		std::vector<Ref<FrameBuffer>> ShadowBuffers;
		std::vector<glm::mat4> LightCameras;
		std::vector<float> CascadeDistances;
		bool RenderShadows;
	};

	class GeometryPass : public ISceneRenderPass
	{
	public:
		GeometryPass(WeakRef<GraphicsContext> context, const GeometryPassInitInfo& initInfo);
		~GeometryPass();

		void Render(const GeometryPassRenderInfo& renderInfo);
		virtual void Resize(uint32_t width, uint32_t height) override;

		Ref<FrameBuffer> GetFB() const { return mFrameData[GetFrameIndex()].FrameBuffer; }
		Ref<Semaphore> GetSemaphore() const { return mFrameData[GetFrameIndex()].Semaphore; }
		Ref<Fence> GetFence() const { return mFrameData[GetFrameIndex()].Fence; }

	private:
		WeakRef<AssetManager> mAssetManager;

		WeakRef<GraphicsContext> mGraphicsContext;

		Ref<GraphicsShader> mOpaqueGeometryShader;
		Ref<GraphicsShader> mTransparentGeometryShader;
		Ref<GraphicsShader> mEnvironmentMapShader;
		Ref<GraphicsShader> mSelectedEntityShader;
		Ref<ComputeShader> mHighlightShader;

		Ref<RenderPass> mRenderPass;
		Ref<CommandPool> mCommandPool;
		Ref<DescriptorSetLayout> mGeometryDescriptorSetLayout;
		Ref<DescriptorSetLayout> mEnvironmentDescriptorSetLayout;
		Ref<DescriptorSetLayout> mHighlightDSL;

		GuidArray<GPU::GPUMaterial>& mMaterialArray;
		GuidArray<WeakRef<ITexture>>& mTextureArray;

		struct FrameData
		{
			Ref<Fence> Fence;
			Ref<Semaphore> Semaphore;
			
			Ref<FrameBuffer> FrameBuffer;
			
			Ref<CommandBuffer> GeometryCommandBuffer;
			
			Ref<DescriptorSet> GeometryDescriptorSet;
			Ref<DescriptorSet> BindlessTextureDS;
			Ref<DescriptorSet> EnvironmentDS;
			Ref<DescriptorSet> HighlightDS;

			bool ResizeRequired = false;
			uint32_t ResizeWidth;
			uint32_t ResizeHeight;
		};

		std::vector<FrameData> mFrameData;
	};
}