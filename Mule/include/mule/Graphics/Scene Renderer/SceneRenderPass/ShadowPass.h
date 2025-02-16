#pragma once

#include "ISceneRenderPass.h"

#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Execution/CommandPool.h"
#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Camera.h"

#include "Asset/AssetManager.h"

#include "Graphics/Scene Renderer/GPUObjects.h"

namespace Mule
{
	struct ShadowPassInitInfo
	{
		WeakRef<AssetManager> AssetManager;
	};

	class ShadowPass : public ISceneRenderPass
	{
	public:
		ShadowPass(WeakRef<GraphicsContext> context, const ShadowPassInitInfo& initInfo);
		virtual ~ShadowPass();

		void Render(WeakRef<Scene> scene, const std::vector<WeakRef<Semaphore>>& waitSemaphores, Camera camera);
		
		const std::vector<Ref<FrameBuffer>>& GetFB() const { return mFrameData[GetFrameIndex()].FrameBuffers; }
		WeakRef<Semaphore> GetSemaphore() const { return mFrameData[GetFrameIndex()].Semaphore; }
		const std::vector<glm::mat4>& GetLightCameras() const { return mFrameData[GetFrameIndex()].LightCameras; }
		const std::vector<float>& GetCascadeDistances() const { return mFrameData[GetFrameIndex()].CascadeDistances; }
		void Resize(uint32_t width, uint32_t height) override;

		bool DidRender() const { return mFrameData[GetFrameIndex()].DidRender; }

		uint32_t GetMaxCascadeCount() const { return MAX_CASCADES; }
		uint32_t GetCascadeCount() const { return mFrameData[GetFrameIndex()].CascadeCount; }
		void SetCascadeCount(uint32_t count);
		void SetCascadeSize(uint32_t width, uint32_t height);

		float zMult = 5.f;
	private:
		const uint32_t MAX_CASCADES = 10;
		WeakRef<GraphicsContext> mGraphicsContext;
		WeakRef<AssetManager> mAssetManager;

		Ref<RenderPass> mRenderPass;
		Ref<CommandPool> mCommandPool;
		Ref<GraphicsShader> mShader;

		struct FrameData
		{
			std::vector<Ref<FrameBuffer>> FrameBuffers;
			Ref<Fence> Fence;
			Ref<Semaphore> Semaphore;
			Ref<CommandBuffer> CommandBuffer;
			Ref<UniformBuffer> CameraBuffer;
			std::vector<glm::mat4> LightCameras;
			std::vector<float> CascadeDistances;

			bool DidRender = false;

			bool ResizeRequired = false;
			uint32_t ResizeWidth, ResizeHeight;
			uint32_t Width, Height;

			bool ChangeCascadeCount = false;
			uint32_t NewCascadeCount;
			uint32_t CascadeCount;
		};

		std::vector<FrameData> mFrameData;

		std::vector<GPU::GPUCamera> mLightCameras;

		std::array<glm::vec3, 8> GetFrustumCornersWorldSpace(const glm::mat4& view, float nearClip, float farClip, float fov, float aspectRatio);
	};
}