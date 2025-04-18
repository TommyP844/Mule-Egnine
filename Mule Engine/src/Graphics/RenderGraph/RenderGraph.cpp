#include "Graphics/RenderGraph/RenderGraph.h"

#include "Graphics/RenderGraph/Resource.h"
#include "Timer.h"

#include <spdlog/spdlog.h>

namespace Mule::RenderGraph
{
	RenderGraph::RenderGraph(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager)
		:
		mIsValid(false),
		mGraphicsContext(context),
		mFrameIndex(0),
		mFrameCount(2), // TODO: We should be getting this from graphics context
		mAssetManager(assetManager)
	{
		mPerFrameData.resize(mFrameCount);
		mResources.resize(mFrameCount);

		mCommandQueue = mGraphicsContext->GetGraphicsQueue();
		mCommandPool = mCommandQueue->CreateCommandPool();

		for (uint32_t i = 0; i < mFrameCount; i++)
		{
			mPerFrameData[i].LayoutTransitionCommandBuffer = mCommandPool->CreateCommandBuffer();
			mPerFrameData[i].LayoutTransitionFence = mGraphicsContext->CreateFence();
			mPerFrameData[i].RenderingFinishedSemaphore = mGraphicsContext->CreateSemaphore();
		}
	}
	
	RenderGraph::~RenderGraph()
	{
	}
	
	const PassContext& RenderGraph::GetPassContext() const
	{
		return mPerFrameData[mFrameIndex].Ctx;
	}

	WeakRef<FrameBuffer> RenderGraph::GetCurrentFrameBuffer() const
	{
		return mPerFrameData[mFrameIndex].Framebuffer;
	}

	void RenderGraph::SetCamera(const Camera& camera)
	{
		for (uint32_t i = 0; i < mFrameCount; i++)
		{
			mPerFrameData[i].Ctx.SetCamera(camera);
		}
	}

	void RenderGraph::Resize(uint32_t width, uint32_t height)
	{
		mPerFrameData[mFrameIndex].Framebuffer->Resize(width, height);
	}

	void RenderGraph::CreateFramebuffer(const FramebufferDescription& desc)
	{
		for (uint32_t i = 0; i < mFrameCount; i++)
		{
			auto framebuffer = mGraphicsContext->CreateFrameBuffer(desc);
			mPerFrameData[i].Framebuffer = framebuffer;
		}
	}

	void RenderGraph::SetFinalLayouts(std::vector<std::pair<uint32_t, ImageLayout>> layouts)
	{
		for (uint32_t i = 0; i < mFrameCount; i++)
		{
			mPerFrameData[i].FinalLayouts = layouts;
		}
	}

	void RenderGraph::AddPass(
		const std::string& name, 
		const std::vector<std::string>& dependecies, 
		AssetHandle shaderHandle, 
		std::vector<std::pair<uint32_t, ImageLayout>> requiredLayouts,
		std::function<void(WeakRef<CommandBuffer>, WeakRef<Scene>, WeakRef<GraphicsShader>, const PassContext&)> callback)
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
		renderPass.ShaderHandle = shaderHandle;
		renderPass.RequiredLayouts = requiredLayouts;
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
			ppd.ShaderHandle = pass.ShaderHandle;
			ppd.RequiredLayouts = pass.RequiredLayouts;

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
	void RenderGraph::Execute(WeakRef<Scene> scene, const std::vector<WeakRef<Semaphore>>& waitSemaphores)
	{
		if (!mIsValid)
		{
			SPDLOG_WARN("Invalid Render Graph trying to execute");
			return;
		}

		PerFrameData& perFrameData = mPerFrameData[mFrameIndex];
		PassContext& ctx = perFrameData.Ctx;

		WeakRef<Semaphore> previousPassSemaphore = nullptr;
		bool firstPass = true;
		for (auto& pass : mPassesToExecute)
		{
			Timer passTimer;
			passTimer.Start();

			auto cmd = pass.CommandBuffers[mFrameIndex];
			auto fence = pass.Fences[mFrameIndex];
			auto semaphore = pass.Semaphores[mFrameIndex];
			WeakRef<GraphicsShader> shader = nullptr;

			if (pass.ShaderHandle)
			{
				shader = mAssetManager->GetAsset<GraphicsShader>(pass.ShaderHandle);
				if (shader == nullptr)
				{
					continue;
				}
			}

			fence->Reset();
			cmd->Reset();
			cmd->Begin();

			if (firstPass)
			{
				firstPass = false;
				cmd->ClearFrameBuffer(perFrameData.Framebuffer);
			}

			for (auto [index, layout] : pass.RequiredLayouts)
			{
				cmd->TranistionImageLayout(perFrameData.Framebuffer->GetColorAttachment(index), layout);
			}

			if (pass.ShaderHandle)
			{
				cmd->BeginRenderPass(perFrameData.Framebuffer, shader);
			}

			pass.Callback(cmd, scene, shader, ctx);

			if(pass.ShaderHandle)
				cmd->EndRenderPass();

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
		
		auto cmd = perFrameData.LayoutTransitionCommandBuffer;
		perFrameData.LayoutTransitionFence->Wait();
		perFrameData.LayoutTransitionFence->Reset();
		cmd->Reset();
		cmd->Begin();
		for (auto [index, layout] : perFrameData.FinalLayouts)
		{
			cmd->TranistionImageLayout(perFrameData.Framebuffer->GetColorAttachment(index), layout);
		}
		cmd->End();

		mCommandQueue->Submit(cmd, { previousPassSemaphore }, { perFrameData.RenderingFinishedSemaphore }, perFrameData.LayoutTransitionFence);
	}
	
	void RenderGraph::NextFrame()
	{
		mFrameIndex = (mFrameIndex + 1) % mFrameCount;
	}
	
	WeakRef<Semaphore> RenderGraph::GetSemaphore() const
	{
		return mPerFrameData[mFrameIndex].RenderingFinishedSemaphore;
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