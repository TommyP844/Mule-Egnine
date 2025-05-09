#include "Graphics/Renderer/RenderGraph/RenderGraph.h"
#include "Graphics/Renderer/RenderGraph/RenderPasses/RenderPass.h"

#include "Graphics/Renderer/Renderer.h"

#include "ECS/Scene.h"

#include "Timer.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	RenderGraph::RenderGraph()
		:
		mIsBaked(false)
	{
		mQueue = GraphicsQueue::Create();
	}

	RenderGraph::~RenderGraph()
	{

	}

	void RenderGraph::InitializeRegistry(ResourceRegistry& registry)
	{
		assert(mIsBaked && "Render Graph must be baked before calling InitializeRegistry()");

		for (auto pass : mPasses)
		{
			const std::string& name = pass->GetName();
			ResourceHandle fenceHandle = registry.AddFence(name + ".Fence");
			ResourceHandle cmdHandle = registry.AddCommandBuffer(name + ".Cmd");

			pass->SetFenceHandle(fenceHandle);
			pass->SetCommandBufferHandle(cmdHandle);

			for (uint32_t i = 0; i < registry.GetFramesInFlight(); i++)
			{
				pass->SetFrameIndex(i);
				pass->Setup(registry);
			}

			// We most likly dont need this because it will we set by the graph before execution
			pass->SetFrameIndex(0);
		}
	}

	void RenderGraph::Bake()
	{
		std::set<std::string> compiledPasses;

		std::unordered_map<std::string, uint32_t> passIndices;

		bool foundPass = true;
		while (!mPassesToCompile.empty())
		{
			assert(foundPass && "RenderGraph Cycle detected");
			foundPass = false;

			for (auto iter = mPassesToCompile.begin(); iter != mPassesToCompile.end(); iter++)
			{
				auto pass = *iter;
				std::vector<std::string> dependencies = pass->GetDependencies();

				dependencies.erase(
					std::remove_if(dependencies.begin(), dependencies.end(),
						[&compiledPasses](const std::string& s) {
							return compiledPasses.contains(s);
						}),
					dependencies.end()
				);

				if (dependencies.empty())
				{
					mPasses.push_back(pass);
					passIndices[pass->GetName()] = mPasses.size() - 1;

					compiledPasses.insert(pass->GetName());

					mPassesToCompile.erase(iter);
					foundPass = true;
					break;
				}
			}
		}

		SPDLOG_INFO("RenderGraph Compiled");

		mIsBaked = true;
	}

	void RenderGraph::Execute(const std::vector<RenderCommand>& commands, const Camera& camera, uint32_t frameIndex)
	{
		assert(mIsBaked && "Render Graph must be baked before calling Execute");

		Ref<ResourceRegistry> registry = camera.GetRegistry();

		if (!registry)
		{
			SPDLOG_WARN("Camera Resource Registry not initialized");
			return;
		}

		registry->WaitForFences(frameIndex);

		if (mPreExecutionCallback)
			mPreExecutionCallback(camera, frameIndex);

		for (auto pass : mPasses)
		{
			auto fence = registry->GetResource<Fence>(pass->GetFenceHandle(), frameIndex);
			auto commandBuffer = registry->GetResource<CommandBuffer>(pass->GetCommandBufferHandle(), frameIndex);

			fence->Wait();
			fence->Reset();

			commandBuffer->Reset();
			commandBuffer->Begin();

			//TODO: insert memory barriers
			pass->SetFrameIndex(frameIndex);
			pass->Render(commandBuffer, commands, *registry);

			commandBuffer->End();

			mQueue->Submit(commandBuffer, {}, {}, fence);
		}
	}
}