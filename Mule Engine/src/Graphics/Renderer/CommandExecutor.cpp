#include "Graphics/Renderer/CommandExecutor.h"

namespace Mule::CommandExecutor
{
	void ExecuteClearFramebufferCommand(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex);
	void ExecuteTransitionLayoutCommand(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex);
	void ExecuteBeginRenderingCommand(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex);
	void ExecuteEndRenderingCommand(Ref<CommandBuffer> cmd);
	void ExecuteBindGraphicsPipeline(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex);
	void ExecuteBindComputePipeline(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex);
	void ExecuteClearRenderTarget(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex);

	void Execute(Ref<CommandBuffer> cmd, const CommandList& commandList, const ResourceRegistry& registry, uint32_t frameIndex)
	{
		for (const RenderCommand& command : commandList.GetCommands())
		{
			switch (command.GetType())
			{
			case RenderCommandType::ClearFramebuffer:
				ExecuteClearFramebufferCommand(cmd, command, registry, frameIndex);
				break;
			
			case RenderCommandType::TransitionLayout:
				ExecuteTransitionLayoutCommand(cmd, command, registry, frameIndex);
				break;

			case RenderCommandType::BeginRendering:
				ExecuteBeginRenderingCommand(cmd, command, registry, frameIndex);
				break;

			case RenderCommandType::EndRendering:
				ExecuteEndRenderingCommand(cmd);
				break;

			case RenderCommandType::BindGraphicsPipeline:
				ExecuteBindGraphicsPipeline(cmd, command, registry, frameIndex);
				break;

			case RenderCommandType::BindComputePipeline:
				ExecuteBindComputePipeline(cmd, command, registry, frameIndex);
				break;

			case RenderCommandType::ClearRenderTarget:
				ExecuteClearRenderTarget(cmd, command, registry, frameIndex);
				break;

			default:
				assert(false && "Invalid Command");
				break;
			}
		}
	}

	void ExecuteClearFramebufferCommand(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex)
	{
		const ClearFramebufferCommand& clearCommand = command.GetCommand<ClearFramebufferCommand>();
		cmd->ClearFrameBuffer(registry.GetResource<Framebuffer>(clearCommand.FramebufferHandle, frameIndex));
	}

	void ExecuteTransitionLayoutCommand(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex)
	{
		const TransitionLayoutCommand& transitionCommand = command.GetCommand<TransitionLayoutCommand>();
		auto texture = registry.GetResource<Texture>(transitionCommand.TextureHandle, frameIndex);
		cmd->TranistionImageLayout(texture, transitionCommand.NewLayout);
	}
	
	void ExecuteBeginRenderingCommand(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex)
	{
		const BeginRenderingCommand& beginCommand = command.GetCommand<BeginRenderingCommand>();

		std::vector<BeginRenderingAttachment> colorAttachments(beginCommand.ColorAttachments.size());
		BeginRenderingAttachment depthAttachment;

		for (uint32_t i = 0; i < beginCommand.ColorAttachments.size(); i++)
		{
			const BeginRenderingCommandAttachment& attachment = beginCommand.ColorAttachments[i];

			colorAttachments[i] = {
				registry.GetResource<Texture>(attachment.AttachmentHandle, frameIndex),
				attachment.ClearOnLoad
			};
		}

		if (beginCommand.DepthAttachment.AttachmentHandle)
		{
			depthAttachment.Attachment = registry.GetResource<Texture>(beginCommand.DepthAttachment.AttachmentHandle, frameIndex);
			depthAttachment.ClearOnLoad = beginCommand.DepthAttachment.ClearOnLoad;
		}

		cmd->BeginRendering(
			colorAttachments,
			depthAttachment
		);
	}

	void ExecuteEndRenderingCommand(Ref<CommandBuffer> cmd)
	{
		cmd->EndRendering();
	}

	void ExecuteBindGraphicsPipeline(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex)
	{
		const BindGraphicsPipelineCommand& bindPipeline = command.GetCommand<BindGraphicsPipelineCommand>();

		std::vector<WeakRef<ShaderResourceGroup>> groups(bindPipeline.ShaderResourceGroups.size());

		for (uint32_t i = 0; i < groups.size(); i++)
		{
			groups[i] = registry.GetResource<ShaderResourceGroup>(bindPipeline.ShaderResourceGroups[i], frameIndex);
		}

		cmd->BindPipeline(bindPipeline.Pipeline, groups);
	}

	void ExecuteBindComputePipeline(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex)
	{
		const BindComputePipelineCommand& bindPipeline = command.GetCommand<BindComputePipelineCommand>();

		std::vector<WeakRef<ShaderResourceGroup>> groups(bindPipeline.ShaderResourceGroups.size());

		for (uint32_t i = 0; i < groups.size(); i++)
		{
			groups[i] = registry.GetResource<ShaderResourceGroup>(bindPipeline.ShaderResourceGroups[i], frameIndex);
		}

		cmd->BindComputePipeline(bindPipeline.Pipeline, groups);
	}

	void ExecuteClearRenderTarget(Ref<CommandBuffer> cmd, const RenderCommand& command, const ResourceRegistry& registry, uint32_t frameIndex)
	{
		const auto& clearCommand = command.GetCommand<ClearRenderTargetCommand>();
		auto renderTarget = registry.GetResource<Texture>(clearCommand.ClearTarget, frameIndex);

		cmd->ClearTexture(renderTarget);
	}
}