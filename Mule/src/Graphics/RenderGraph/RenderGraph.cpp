#include "Graphics/RenderGraph/RenderGraph.h"

#include "Graphics/RenderGraph/Resource.h"

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

	void RenderGraph::AddPass(const std::string& name, const std::vector<std::string>& inputs, const std::vector<std::string>& outputs, std::function<void(const PassContext&)> callback, bool hasCommands)
	{
		auto iter = mPassesToCompile.find(name);
		if (iter != mPassesToCompile.end())
		{
			SPDLOG_WARN("RenderGraph Pass already exists: {}", name);
			return;
		}
		
		RenderPassInfo& renderPass = mPassesToCompile[name];
		renderPass.Inputs = inputs;
		renderPass.Outputs = outputs;
		renderPass.CallBack = callback;
		renderPass.HasCommands = hasCommands;
	}
	
	void RenderGraph::Compile()
	{
		for (int i = 0; i < mFrameCount; i++)
		{
			mPerFrameData[i].Ctx = PassContext(mResources[i]);
		}

		// TODO: use some algorithm that i dont know how to implement to sort/filter the passes
		for (auto& [name, pass] : mPassesToCompile)
		{
			PerPassData ppd{};

			ppd.Callback = pass.CallBack;

			ppd.CommandBuffers.resize(mFrameCount);
			ppd.Semaphores.resize(mFrameCount);
			ppd.Fences.resize(mFrameCount);

			if (pass.HasCommands)
			{
				ppd.HasCommands = true;
				for (uint32_t i = 0; i < mFrameCount; i++)
				{
					ppd.CommandBuffers[i] = mCommandPool->CreateCommandBuffer();
					ppd.Fences[i] = mGraphicsContext->CreateFence();
					ppd.Semaphores[i] = mGraphicsContext->CreateSemaphore();
				}
			}

			mPassesToExecute.push_back(ppd);
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

		for (auto& pass : mPassesToExecute)
		{
			if (pass.HasCommands)
			{
				pass.Fences[mFrameIndex]->Wait();
				pass.Fences[mFrameIndex]->Reset();
			}
		}

		std::vector<WeakRef<Semaphore>> waits = waitSemaphores;

		for (auto& pass : mPassesToExecute)
		{
			auto cmd = pass.CommandBuffers[mFrameIndex];
			auto fence = pass.Fences[mFrameIndex];
			auto semaphore = pass.Semaphores[mFrameIndex];

			if (pass.HasCommands)
			{
				ctx.SetCommandBuffer(cmd);

				cmd->Reset();
				cmd->Begin();
			}
			else
			{
				ctx.SetCommandBuffer(nullptr);
			}

			pass.Callback(ctx);

			if (pass.HasCommands)
			{
				cmd->End();
				mCommandQueue->Submit(cmd, waits, { semaphore }, fence);
				waits.push_back(semaphore);
			}
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
}