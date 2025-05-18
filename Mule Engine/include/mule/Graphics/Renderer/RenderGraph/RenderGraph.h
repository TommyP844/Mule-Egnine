#pragma once

#include "Ref.h"

#include "Services/ServiceManager.h"

#include "Graphics/API/GraphicsQueue.h"
#include "Graphics/API/CommandAllocator.h"

#include "Graphics/Renderer/RenderCommand.h"

#include "Graphics/Renderer/RenderGraph/RenderPass.h"
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
		void Execute(const CommandList& commands, const Camera& camera, uint32_t frameIndex);

		WeakRef<RenderPass> CreatePass(const std::string& name, PassType type);
		
		void SetPreExecutionCallback(std::function<void(const Camera&, const CommandList&, uint32_t)> callback) { mPreExecutionCallback = callback; }
		void SetResizeCallback(std::function<void(const Camera&, uint32_t, uint32_t, uint32_t)> callback) { mResizeCallback = callback; }
		void SetRegistrySetupCallback(std::function<void(const ResourceRegistry&, uint32_t frameIndex)> callback) { mSetupCallback = callback; }

	private:
		bool mIsBaked = false;
		Ref<GraphicsQueue> mQueue;
		std::vector<Ref<RenderPass>> mPasses;

		std::function<void(const Camera&, const CommandList&, uint32_t)> mPreExecutionCallback;
		std::function<void(const Camera&, uint32_t, uint32_t, uint32_t)> mResizeCallback;
		std::function<void(const ResourceRegistry&, uint32_t frameIndex)> mSetupCallback;

	};
}
