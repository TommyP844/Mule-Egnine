#pragma once

#include "RenderGraph/RenderGraph.h"

#include "Services/ServiceManager.h"

// STD
#include <vector>
#include <mutex>

#ifdef min
#undef min
#endif

namespace Mule
{
	class SceneRenderer : public RenderGraph::RenderGraph
	{
	public:
		SceneRenderer(Ref<ServiceManager> serviceManager);
		virtual ~SceneRenderer();
		
		Ref<Framebuffer> GetCurrentFrameBuffer() const override;

		

	private:
		Ref<ServiceManager> mServiceManager;

		Mule::RenderGraph::ResourceHandle<Framebuffer> mMainFramebufferHandle;
		Mule::RenderGraph::ResourceHandle<ShaderResourceGroup> mBindlessTextureShaderResourceHandle;

		Ref<ShaderResourceBlueprint> mBindlessShaderResourceBlueprint;

		void PreRenderCallback(Ref<CommandBuffer> commandBuffer);
		void PostRenderCallback(Ref<CommandBuffer> commandBuffer);
		void ResizeCallback(uint32_t width, uint32_t height);
	};
}