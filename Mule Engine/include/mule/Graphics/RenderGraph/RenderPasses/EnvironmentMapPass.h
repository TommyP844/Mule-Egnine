#pragma once

#include "Graphics/RenderGraph/IRenderPass.h"

namespace Mule
{
	class EnvironmentMapPass : public RenderGraph::IRenderPass
	{
	public:
		EnvironmentMapPass(Ref<ServiceManager> serviceManager, WeakRef<RenderGraph::RenderGraph> graph);

		void Setup() override;
		bool Validate() override;
		void Render(Ref<CommandBuffer> cmd, WeakRef<Scene> scene) override;
		void Resize(uint32_t width, uint32_t height) override;

		// Inputs
		void SetCameraBufferHandle(Mule::RenderGraph::ResourceHandle<UniformBuffer> cameraBufferHandle);
		void SetFramebufferHandle(Mule::RenderGraph::ResourceHandle<Framebuffer> framebufferHandle);

	private:
		Ref<ServiceManager> mServiceManager;

		// Pass Generated
		Mule::RenderGraph::ResourceHandle<ShaderResourceGroup> mResourceGroup;

		// Inputs
		Mule::RenderGraph::ResourceHandle<UniformBuffer> mCameraBufferHandle;
		Mule::RenderGraph::ResourceHandle<Framebuffer> mFramebufferHandle;
	};
}