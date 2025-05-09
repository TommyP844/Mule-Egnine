#pragma once

#include "Ref.h"

#include "Services/ServiceManager.h"

#include "Graphics/API/GraphicsQueue.h"
#include "Graphics/API/CommandAllocator.h"

#include "Graphics/Renderer/RenderCommand.h"

#include "Graphics/Renderer/RenderGraph/RenderPasses/RenderPass.h"
#include "Graphics/Renderer/RenderGraph/ResourceBuilder.h"

#include "Graphics/Camera.h"

#include <vector>

namespace Mule
{
	class RenderGraph
	{
	public:
		RenderGraph();
		virtual ~RenderGraph();

		void InitializeRegistry(ResourceRegistry& registry);

		void Bake();
		void Execute(const std::vector<RenderCommand>& commands, const Camera& camera, uint32_t frameIndex);

		template<typename T, typename ... Args>
		WeakRef<T> CreatePass(ResourceBuilder& resourceBuilder, Args&&... args);

		void SetPreExecutionCallback(std::function<void(const Camera&, uint32_t)> callback) { mPreExecutionCallback = callback; }

	private:
		bool mIsBaked = false;
		Ref<GraphicsQueue> mQueue;
		std::vector<Ref<RenderPass>> mPassesToCompile;
		std::vector<Ref<RenderPass>> mPasses;

		std::function<void(const Camera&, uint32_t)> mPreExecutionCallback;
	};
}

#include "RenderGraph.inl"