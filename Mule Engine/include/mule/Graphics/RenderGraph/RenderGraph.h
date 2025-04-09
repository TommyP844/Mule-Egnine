#pragma once

#include "PassContext.h"
#include "RenderPassStats.h"

#include "WeakRef.h"
#include "Ref.h"

#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Execution/GraphicsQueue.h"
#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Execution/CommandPool.h"
#include "Graphics/Execution/Semaphore.h"
#include "Graphics/Execution/Fence.h"
#include "ECS/Scene.h"
#include "Asset/AssetManager.h"

#include <string>
#include <vector>
#include <functional>

namespace Mule::RenderGraph
{
	class IResource;

	class RenderGraph
	{
	public:
		RenderGraph() = default;
		RenderGraph(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager);
		~RenderGraph();

		template<typename T>
		void AddResource(uint32_t frameIndex, const std::string& name, Ref<T> resource);

		template<typename T>
		WeakRef<T> GetResource(uint32_t frameIndex, const std::string& name) const;

		const PassContext& GetPassContext() const;

		WeakRef<FrameBuffer> GetCurrentFrameBuffer() const;
		void SetCamera(const Camera& camera);
		void Resize(uint32_t width, uint32_t height);
		void CreateFramebuffer(const FramebufferDescription& desc);
		void SetFinalLayouts(std::vector<std::pair<uint32_t, ImageLayout>> layouts);
		void AddPass(
			const std::string& name, 
			const std::vector<std::string>& dependecies, 
			AssetHandle shaderHandle,
			std::vector<std::pair<uint32_t, ImageLayout>> requiredLayouts,
			std::function<void(WeakRef<CommandBuffer>, WeakRef<Scene>, WeakRef<GraphicsShader>, const PassContext&)> func);
		void Compile();
		void Execute(WeakRef<Scene> scene, const std::vector<WeakRef<Semaphore>>& waitSemaphores);
		void NextFrame();

		// Querys resource from the pass context used in the render passes
		template<typename T>
		WeakRef<T> QueryResource(const std::string& name) const;

		// Returns the semaphore from the last render pass
		WeakRef<Semaphore> GetSemaphore() const;

		bool IsValid() const { return mIsValid; }
		uint32_t GetFrameCount() const { return mFrameCount; }
		uint32_t GetFrameIndex() const { return mFrameIndex; }

		void Wait();

		std::vector<RenderPassStats> GetRenderPassStats() const;

	private:
		WeakRef<GraphicsContext> mGraphicsContext;
		WeakRef<AssetManager> mAssetManager;
		Ref<CommandPool> mCommandPool;
		WeakRef<GraphicsQueue> mCommandQueue;

		struct RenderPassInfo
		{
			AssetHandle ShaderHandle = NullAssetHandle;
			std::vector<std::string> Dependecies = {};
			std::function<void(WeakRef<CommandBuffer>, WeakRef<Scene>, WeakRef<GraphicsShader>, const PassContext&)> CallBack = nullptr;
			std::vector<std::pair<uint32_t, ImageLayout>> RequiredLayouts = {};
		};

		struct PerFrameData
		{
			PassContext Ctx;
			Ref<FrameBuffer> Framebuffer;
			std::vector<std::pair<uint32_t, ImageLayout>> FinalLayouts = {};
			Ref<CommandBuffer> LayoutTransitionCommandBuffer;
			Ref<Fence> LayoutTransitionFence;
			Ref<Semaphore> RenderingFinishedSemaphore;
		};

		struct PerPassData
		{
			std::function<void(WeakRef<CommandBuffer>, WeakRef<Scene>, WeakRef<GraphicsShader>, const PassContext&)> Callback;
			std::vector<Ref<CommandBuffer>> CommandBuffers;
			std::vector<Ref<Semaphore>> Semaphores;
			std::vector<Ref<Fence>> Fences;
			RenderPassStats Stats;
			AssetHandle ShaderHandle;
			std::vector<std::pair<uint32_t, ImageLayout>> RequiredLayouts = {};
		};

		std::vector<std::unordered_map<std::string, Ref<IResource>>> mResources;
		std::unordered_map<std::string, RenderPassInfo> mPassesToCompile;
		std::vector<PerPassData> mPassesToExecute;
		std::vector<PerFrameData> mPerFrameData;
		uint32_t mFrameCount, mFrameIndex;
		bool mIsValid;
	};
}

#include "RenderGraph.inl"