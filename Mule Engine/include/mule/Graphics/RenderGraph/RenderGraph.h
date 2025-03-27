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
		RenderGraph(WeakRef<GraphicsContext> context);
		~RenderGraph();

		template<typename T>
		void AddResource(uint32_t frameIndex, const std::string& name, Ref<T> resource);

		template<typename T>
		WeakRef<T> GetResource(uint32_t frameIndex, const std::string& name) const;

		void SetCamera(const Camera& camera);
		void SetScene(WeakRef<Scene> scene);

		void AddPass(const std::string& name, const std::vector<std::string>& dependecies, std::function<void(const PassContext&)> func);
		void Compile();
		void Execute(const std::vector<WeakRef<Semaphore>>& waitSemaphores);
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
		Ref<CommandPool> mCommandPool;
		WeakRef<GraphicsQueue> mCommandQueue;

		struct RenderPassInfo
		{
			std::vector<std::string> Dependecies = {};
			std::function<void(const PassContext&)> CallBack = nullptr;
		};

		struct PerFrameData
		{
			PassContext Ctx;
		};

		struct PerPassData
		{
			std::function<void(const PassContext&)> Callback;
			std::vector<Ref<CommandBuffer>> CommandBuffers;
			std::vector<Ref<Semaphore>> Semaphores;
			std::vector<Ref<Fence>> Fences;
			RenderPassStats Stats;
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