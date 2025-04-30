#include "Graphics/SceneRenderer.h"

#include "Graphics/VertexLayout.h"
#include "Graphics/Material.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/API/GraphicsCore.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/API/CommandAllocator.h"
#include "Graphics/API/UniformBuffer.h"

#include "ECS/Components.h"
#include "Timer.h"

#include "Engine Context/EngineAssets.h"


// Passes
#include "Graphics/RenderGraph/RenderPasses/SolidGeometryPass.h"

namespace Mule
{	
	SceneRenderer::SceneRenderer(Ref<ServiceManager> serviceManager)
		:
		mServiceManager(serviceManager),
		RenderGraph::RenderGraph(serviceManager)
	{
		SPDLOG_INFO("Scene renderer created");

		using FramebufferHandle = Mule::RenderGraph::ResourceHandle<Framebuffer>;
		using UniformBufferHandle = Mule::RenderGraph::ResourceHandle<UniformBuffer>;
		using ShaderResourceGroupHandle = Mule::RenderGraph::ResourceHandle<ShaderResourceGroup>;

		// Uniform Buffers
		UniformBufferHandle cameraBufferHandle = AddResource<UniformBuffer>(sizeof(Camera));

		// Framebuffers
		FramebufferDescription mainFramebufferDesc{};
		mainFramebufferDesc.Layers = 1;
		mainFramebufferDesc.Width = 800;
		mainFramebufferDesc.Height = 600;
		mainFramebufferDesc.ColorAttachments = {
			FramebufferAttachment(TextureFormat::RGBA_32F)
		};
		mainFramebufferDesc.DepthAttachment = FramebufferAttachment(TextureFormat::D_32F);

		mMainFramebufferHandle = AddResource<Framebuffer>(mainFramebufferDesc);

		// Shader Resource Groups
		mBindlessShaderResourceBlueprint = ShaderResourceBlueprint::Create({
			ShaderResourceDescription(0, ShaderResourceType::Sampler, ShaderStage::Fragment, 4096)
			});
		std::vector<Ref<ShaderResourceBlueprint>> blueprints = {
			mBindlessShaderResourceBlueprint
		};
		mBindlessTextureShaderResourceHandle = AddResource<ShaderResourceGroup>(blueprints);

		// Geometry Pass
		{
			auto solidGeometryPass = CreatePass<SolidGeometryPass>();
			solidGeometryPass->SetCameraBufferHandle(cameraBufferHandle);
			solidGeometryPass->SetFramebufferHandle(mMainFramebufferHandle);
			solidGeometryPass->SetBindlessTextureResourceHandle(mBindlessTextureShaderResourceHandle);
		}

		SetPreRenderCallback(std::bind(&SceneRenderer::PreRenderCallback, this, std::placeholders::_1));
		SetPostRenderCallback(std::bind(&SceneRenderer::PostRenderCallback, this, std::placeholders::_1));
		SetResizeCallback(std::bind(&SceneRenderer::ResizeCallback, this, std::placeholders::_1, std::placeholders::_2));

		Bake();
	}

	SceneRenderer::~SceneRenderer()
	{
		SPDLOG_INFO("Scene renderer shutdown");
	}

	Ref<Framebuffer> SceneRenderer::GetCurrentFrameBuffer() const
	{
		return GetResource(mMainFramebufferHandle);
	}

	void SceneRenderer::PreRenderCallback(Ref<CommandBuffer> commandBuffer)
	{
		// setup buffers
	}

	void SceneRenderer::PostRenderCallback(Ref<CommandBuffer> commandBuffer)
	{
		auto framebuffer = GetResource(mMainFramebufferHandle);
		commandBuffer->TranistionImageLayout(framebuffer->GetColorAttachment(0), ImageLayout::ShaderReadOnly);
	}

	void SceneRenderer::ResizeCallback(uint32_t width, uint32_t height)
	{
		SPDLOG_INFO("SceneRenderer Resize Callback: {}x{}", width, height);

		auto framebuffer = GetResource(mMainFramebufferHandle);
		framebuffer->Resize(width, height);
	}
}