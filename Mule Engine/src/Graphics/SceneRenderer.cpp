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

		// Geometry Pass
		{
			auto solidGeometryPass = CreatePass<SolidGeometryPass>();
			solidGeometryPass->SetCameraBufferHandle(cameraBufferHandle);
			solidGeometryPass->SetFramebufferHandle(mMainFramebufferHandle);
		}

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
}