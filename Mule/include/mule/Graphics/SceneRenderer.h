#pragma once

// Engine
#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Execution/Semaphore.h"
#include "Graphics/Execution/Fence.h"
#include "Graphics/RenderPass.h"
#include "Context/GraphicsContext.h"
#include "ECS/Scene.h"
#include "Asset/AssetManager.h"

// STD
#include <array>

namespace Mule
{
	class SceneRenderer
	{
	public:
		SceneRenderer(Ref<GraphicsContext> context, Ref<AssetManager> assetManager);
		~SceneRenderer();

		void Render(Ref<Scene> scene, std::vector<Ref<Semaphore>> waitSemaphore = {});
		Ref<Semaphore> GetCurrentFrameRenderFinishedSemaphore() const { return mFrameData[mFrameIndex].RenderingFinishedSemaphore; }
		Ref<FrameBuffer> GetCurrentFrameBuffer() const { return mFrameData[mFrameIndex].Framebuffer; }
		void Resize(uint32_t width, uint32_t height);

	private:
		Ref<GraphicsContext> mGraphicsContext;
		Ref<AssetManager> mAssetManager;
		bool mIsValid;

		struct FrameData
		{
			Ref<FrameBuffer> Framebuffer;
			Ref<Semaphore> RenderingFinishedSemaphore;
			Ref<Fence> RenderingFinishedFence;
			Ref<CommandPool> CommandPool;
			Ref<CommandBuffer> ShadowPassCmdBuffer;
			Ref<CommandBuffer> SolidGeometryPassCmdBuffer;
			bool ResizeRequired = false;
			uint32_t ResizeWidth = 0;
			uint32_t ResizeHeight = 0;
		};
		std::array<FrameData, 2> mFrameData;
		uint32_t mFrameIndex;

		Ref<RenderPass> mMainRenderPass;
		Ref<GraphicsShader> mDefaultGeometryShader;

	};
}