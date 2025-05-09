#include "Graphics/Renderer/RenderGraph/RenderPasses/GBufferPass.h"

#include "Graphics/ShaderFactory.h"

#include "Graphics/Material.h"
#include "Graphics/GPUObjects.h"

namespace Mule
{
	GBufferPass::GBufferPass(ResourceBuilder& resourceBuilder)
		:
		RenderPass("Solid Geometry Pass", PassType::Graphics)
	{
		// Buffers
		mCameraBufferHandle = resourceBuilder.CreateResource("CameraBuffer", ResourceType::UniformBuffer, (uint32_t)sizeof(GPU::Camera));

		// SRGs
		mCameraSRGHandle = resourceBuilder.CreateResource("CameraSRG", {
			ShaderResourceDescription(0, ShaderResourceType::UniformBuffer, ShaderStage::Vertex)
			});

		mBindlessMaterialSRGHandle = resourceBuilder.GetGlobalResource("Bindless.Material.SRG");
		mBindlessTextureSRGHandle = resourceBuilder.GetGlobalResource("Bindless.Texture.SRG");

		mFramebufferHandle = resourceBuilder.CreateResource("GBuffer", FramebufferDescription{
			.Width = 800,
			.Height = 600,
			.ColorAttachments = {
				{ TextureFormat::RGBA_32F, TextureFlags::None }, // Albedo
				{ TextureFormat::RGBA_32F, TextureFlags::None }, // Normal
				{ TextureFormat::RGBA_32F, TextureFlags::None }, // PBR Factors
			},
			.DepthAttachment = { TextureFormat::D_32F, TextureFlags::None },
			});


		mResourceUsage = {
			{ mCameraBufferHandle, ResourceUsage::Read },
			{ mCameraSRGHandle, ResourceUsage::Read },
			{ mFramebufferHandle, ResourceUsage::Write }
		};
	}

	void GBufferPass::Setup(ResourceRegistry& registry)
	{
		auto cameraBuffer = registry.GetResource<UniformBuffer>(mCameraBufferHandle, GetFrameIndex());
		auto cameraSRG = registry.GetResource<ShaderResourceGroup>(mCameraSRGHandle, GetFrameIndex());

		cameraSRG->Update(0, cameraBuffer);
	}

	void GBufferPass::Resize(ResourceRegistry& registry, uint32_t width, uint32_t height)
	{
	}

	void GBufferPass::Render(Ref<CommandBuffer> cmd, const std::vector<RenderCommand>& commands, const ResourceRegistry& registry)
	{
		WeakRef<GraphicsPipeline> pipeline = ShaderFactory::Get().GetOrCreateGraphicsPipeline("Geometry");
		Ref<Framebuffer> framebuffer = registry.GetResource<Framebuffer>(mFramebufferHandle, GetFrameIndex());
		Ref<ShaderResourceGroup> cameraSRG = registry.GetResource<ShaderResourceGroup>(mCameraSRGHandle, GetFrameIndex());
		Ref<ShaderResourceGroup> bindlessTextureSRG = registry.GetResource<ShaderResourceGroup>(mBindlessTextureSRGHandle, GetFrameIndex());
		Ref<ShaderResourceGroup> bindlessMaterialSRG = registry.GetResource<ShaderResourceGroup>(mBindlessMaterialSRGHandle, GetFrameIndex());

		cmd->ClearFrameBuffer(framebuffer);
		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ColorAttachment);
		cmd->BeginRendering(framebuffer, pipeline, { cameraSRG, bindlessTextureSRG, bindlessMaterialSRG });

		for (auto command : commands)
		{
			if (command.GetType() != RenderCommandType::Draw)
				continue;
			
			auto& drawCommand = command.GetCommand<DrawCommand>();
			auto& material = drawCommand.Material;
			
			uint32_t materialIndex = 0; // material->GlobalIndex;

			cmd->SetPushConstants(pipeline, ShaderStage::Vertex, &drawCommand.ModelMatrix[0][0], sizeof(drawCommand.ModelMatrix));
			cmd->SetPushConstants(pipeline, ShaderStage::Fragment, &materialIndex, sizeof(materialIndex));
			cmd->BindAndDrawMesh(drawCommand.Mesh, 1);
		}

		cmd->EndRendering();

		cmd->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly);
	}
}
