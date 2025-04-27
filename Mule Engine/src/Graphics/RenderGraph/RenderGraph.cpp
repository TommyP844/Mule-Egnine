#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/IRenderPass.h"

#include "ECS/Scene.h"

#include "Timer.h"

#include <spdlog/spdlog.h>

namespace Mule::RenderGraph
{
	RenderGraph::RenderGraph(Ref<ServiceManager> serviceManager)
		:
		mFramesInFlight(2),
		mFrameIndex(0),
		mServiceManager(serviceManager)
	{
		mQueue = GraphicsQueue::Create();
		mCommandAllocator = CommandAllocator::Create();

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			mResizeEvents.push_back(ResizeEvent{});
		}

	}

	RenderGraph::~RenderGraph()
	{

	}

	void RenderGraph::Resize(uint32_t width, uint32_t height)
	{
		for (auto& event : mResizeEvents)
		{
			event.Resize = true;
			event.Width = width;
			event.Height = height;
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
				std::vector<std::string> dependecies = pass->GetDependecies();

				dependecies.erase(
					std::remove_if(dependecies.begin(), dependecies.end(), 
						[&compiledPasses](const std::string& s) {
							return compiledPasses.contains(s);
						}),
					dependecies.end()
				);

				if (dependecies.empty())
				{
					PassInFlight inFlightPass{};

					inFlightPass.Cmd = mCommandAllocator->CreateCommandBuffer();
					inFlightPass.Fence = Fence::Create();
					inFlightPass.Pass = pass;
					//inFlightPass.WaitSemaphore; = Semaphore::Create();
					//inFlightPass.SignalSemaphore; = Semaphore::Create();

					mPasses.push_back(inFlightPass);
					passIndices[pass->GetName()] = mPasses.size() - 1;

					compiledPasses.insert(pass->GetName());

					mPassesToCompile.erase(iter);
					foundPass = true;
					break;
				}
			}
		}

		for (auto& pass : mPasses)
		{
			for (const auto& passDependecy : pass.Pass->GetDependecies())
			{
				uint32_t dependencyIndex = passIndices[passDependecy];
				auto& dependencyPass = mPasses[dependencyIndex];

				auto semaphore = Semaphore::Create();
				dependencyPass.SignalSemaphores.push_back(semaphore);
				pass.WaitSemaphores.push_back(semaphore);
			}
		}

		mPasses.back().SignalSemaphores.push_back(Semaphore::Create());

		SPDLOG_INFO("RenderGraph Compiled");

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			mFrameIndex = i;
			for (auto& pass : mPasses)
			{
				assert(pass.Pass->Validate() && "Failed to validate render pass");
				pass.Pass->Setup();
			}
		}
		mFrameIndex = 0;
	}

	void RenderGraph::SetResizeCallback(std::function<void(uint32_t, uint32_t)> func)
	{
		mResizeCallback = func;
	}

	void RenderGraph::Execute(WeakRef<Scene> scene)
	{
		for (auto pass : mPasses)
		{
			pass.Fence->Wait();
			pass.Fence->Reset();

			if (mResizeEvents[mFrameIndex].Resize)
			{
				ResizeEvent& event = mResizeEvents[mFrameIndex];
				pass.Pass->Resize(event.Width, event.Height);
			}

			pass.Cmd->Reset();
			pass.Cmd->Begin();

			pass.Pass->Render(pass.Cmd, scene);

			pass.Cmd->End();

			mQueue->Submit(pass.Cmd, pass.WaitSemaphores, pass.SignalSemaphores, pass.Fence);
		}

		if (mResizeEvents[mFrameIndex].Resize)
			mResizeEvents[mFrameIndex].Resize = false;

		mFrameIndex ^= 1;
	}

	Ref<Semaphore> RenderGraph::GetCurrentSemaphore()
	{
		return mPasses.back().SignalSemaphores[0];
	}
}