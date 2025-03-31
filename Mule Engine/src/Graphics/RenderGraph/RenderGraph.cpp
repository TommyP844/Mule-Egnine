#include "Graphics/RenderGraph/RenderGraph.h"

#include "Graphics/RenderGraph/Resource.h"
#include "Timer.h"

#include <spdlog/spdlog.h>

namespace Mule::RenderGraph
{
	RenderGraph::RenderGraph(WeakRef<GraphicsContext> context)
		:
		mIsValid(false),
		mGraphicsContext(context),
		mFrameIndex(0),
		mFrameCount(2) // TODO: We should be getting this from graphics context
	{
		mPerFrameData.resize(mFrameCount);
		mResources.resize(mFrameCount);

		mCommandQueue = mGraphicsContext->GetGraphicsQueue();
		mCommandPool = mCommandQueue->CreateCommandPool();
	}
	
	RenderGraph::~RenderGraph()
	{
	}
	
	const PassContext& RenderGraph::GetPassContext() const
	{
		return mPerFrameData[mFrameIndex].Ctx;
	}

	void RenderGraph::SetCamera(const Camera& camera)
	{
		for (uint32_t i = 0; i < mFrameCount; i++)
		{
			mPerFrameData[i].Ctx.SetCamera(camera);
		}
	}

	void RenderGraph::SetScene(WeakRef<Scene> scene)
	{
		for (uint32_t i = 0; i < mFrameCount; i++)
		{
			mPerFrameData[i].Ctx.SetScene(scene);
		}
	}

	void RenderGraph::AddPass(const std::string& name, const std::vector<std::string>& dependecies, std::function<void(const PassContext&)> callback)
	{
		auto iter = mPassesToCompile.find(name);
		if (iter != mPassesToCompile.end())
		{
			SPDLOG_WARN("RenderGraph Pass already exists: {}", name);
			return;
		}
		
		RenderPassInfo& renderPass = mPassesToCompile[name];
		renderPass.Dependecies = dependecies;
		renderPass.CallBack = callback;
	}
	
	void RenderGraph::Compile()
	{
		for (int i = 0; i < mFrameCount; i++)
		{
			mPerFrameData[i].Ctx = PassContext(mResources[i]);
		}

		auto passIter = mPassesToCompile.begin();
		while (true)
		{
			if (passIter == mPassesToCompile.end())
			{
				if (mPassesToCompile.empty())
					break;
				passIter = mPassesToCompile.begin();
			}

			std::string name = passIter->first;
			RenderPassInfo& pass = passIter->second;
			if (!pass.Dependecies.empty())
			{
				passIter++;
				continue;
			}

			PerPassData ppd{};

			ppd.Callback = pass.CallBack;

			ppd.CommandBuffers.resize(mFrameCount);
			ppd.Semaphores.resize(mFrameCount);
			ppd.Fences.resize(mFrameCount);
			ppd.Stats.Name = name;

			for (uint32_t i = 0; i < mFrameCount; i++)
			{
				ppd.CommandBuffers[i] = mCommandPool->CreateCommandBuffer();
				ppd.Fences[i] = mGraphicsContext->CreateFence();
				ppd.Semaphores[i] = mGraphicsContext->CreateSemaphore();
			}

			mPassesToExecute.push_back(ppd);

			passIter = mPassesToCompile.erase(passIter);

			// Remove pass dependecies
			for (auto& [depName, depPass] : mPassesToCompile)
			{
				if (name == depName)
					continue;

				auto iter = std::find(depPass.Dependecies.begin(), depPass.Dependecies.end(), name);
				if (iter == depPass.Dependecies.end())
					continue;

				depPass.Dependecies.erase(iter);
			}
			
		}

		mIsValid = true;
	}
	
	// TODO: handle wait semaphores properly from dependecies
	void RenderGraph::Execute(const std::vector<WeakRef<Semaphore>>& waitSemaphores)
	{
		if (!mIsValid)
		{
			SPDLOG_WARN("Invalid Render Graph trying to execute");
			return;
		}

		PerFrameData& perFrameData = mPerFrameData[mFrameIndex];
		PassContext& ctx = perFrameData.Ctx;

		WeakRef<Semaphore> previousPassSemaphore = nullptr;
		for (auto& pass : mPassesToExecute)
		{
			Timer passTimer;
			passTimer.Start();

			auto cmd = pass.CommandBuffers[mFrameIndex];
			auto fence = pass.Fences[mFrameIndex];
			auto semaphore = pass.Semaphores[mFrameIndex];

			ctx.SetCommandBuffer(cmd);

			fence->Reset();
			cmd->Reset();
			cmd->Begin();

			pass.Callback(ctx);

			cmd->End();
			if (previousPassSemaphore)
			{
				mCommandQueue->Submit(cmd, { previousPassSemaphore }, { semaphore }, fence);
				previousPassSemaphore = semaphore;
			}
			else
			{
				mCommandQueue->Submit(cmd, waitSemaphores, { semaphore }, fence);
				previousPassSemaphore = semaphore;
			}
			
			passTimer.Stop();
			pass.Stats.CPUExecutionTime = passTimer.Query();
		}
		
	}
	void RenderGraph::NextFrame()
	{
		mFrameIndex = (mFrameIndex + 1) % mFrameCount;
	}
	
	WeakRef<Semaphore> RenderGraph::GetSemaphore() const
	{
		return mPassesToExecute.back().Semaphores[mFrameIndex];
	}

	void RenderGraph::Wait()
	{
		for (auto& pass : mPassesToExecute)
		{
			pass.Fences[mFrameIndex]->Wait();
		}
	}
	
	std::vector<RenderPassStats> RenderGraph::GetRenderPassStats() const
	{
		std::vector<RenderPassStats> stats;

		for (auto pass : mPassesToExecute)
		{
			stats.push_back(pass.Stats);
		}

		return stats;
	}
}