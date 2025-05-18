#include "Graphics/Renderer/RenderGraph/RenderPass.h"
#include "Graphics/Renderer/CommandExecutor.h"

#include <assert.h>

namespace Mule
{
	RenderPass::RenderPass(const std::string& name, PassType type)
		:
		mName(name),
		mPassType(type)
	{
		mFenceName = name + ".Fence";
		mCmdName = name + ".Cmd";
		mFenceHandle = ResourceHandle(mFenceName, ResourceType::Fence);
		mCommandBufferHandle = ResourceHandle(mCmdName, ResourceType::CommandBuffer);
	}

	void RenderPass::InitRegistry(ResourceRegistry& registry)
	{
		registry.AddFence(mFenceName);
		registry.AddCommandBuffer(mCmdName);
	}

	void RenderPass::AddPreDrawCommand(const RenderCommand& command)
	{
		mPreDrawCommandList.AddCommand(command);
	}

	void RenderPass::AddPostDrawCommand(const RenderCommand& command)
	{
		mPostDrawCommandList.AddCommand(command);
	}

	void RenderPass::AddDependency(const std::string& passDependency)
	{
		mDependencies.push_back(passDependency);
	}

	Ref<CommandBuffer> RenderPass::Execute(const CommandList& commandList, const ResourceRegistry& registry, uint32_t frameIndex)
	{
		Ref<CommandBuffer> cmd = registry.GetResource<CommandBuffer>(mCommandBufferHandle, frameIndex);
		Ref<Fence> fence = registry.GetResource<Fence>(mFenceHandle, frameIndex);

		fence->Wait();
		fence->Reset();

		cmd->Reset();
		cmd->Begin();

		CommandExecutor::Execute(cmd, mPreDrawCommandList, registry, frameIndex);

		if (mExecutionCallback)
		{
			mExecutionCallback(cmd, commandList, registry, frameIndex);
		}

		CommandExecutor::Execute(cmd, mPostDrawCommandList, registry, frameIndex);

		return cmd;
	}

	Ref<Fence> RenderPass::GetFence(const ResourceRegistry& registry, uint32_t frameIndex)
	{
		Ref<Fence> fence = registry.GetResource<Fence>(mFenceHandle, frameIndex);
		return fence;
	}

	void RenderPass::AddResource(ResourceHandle handle, ResourceAccess access, uint32_t index)
	{
		mResourceUsage[handle] = { access, index };
	}

	void RenderPass::AddCommandType(RenderCommandType type)
	{
		mCommandTypes.insert(type);
	}

	void RenderPass::SetExecutionCallback(std::function<void(Ref<CommandBuffer>, const CommandList&, const ResourceRegistry&, uint32_t)> callback)
	{
		mExecutionCallback = callback;
	}

	void RenderPass::SetPipeline(WeakRef<GraphicsPipeline> pipeline)
	{
		mGraphicsPipeline = pipeline;
	}

	void RenderPass::SetPipeline(WeakRef<ComputePipeline> pipeline)
	{
		mComputePipeline = pipeline;
	}

}
