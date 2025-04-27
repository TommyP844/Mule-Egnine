#pragma once

#include "Ref.h"
#include "Services/ServiceManager.h"
#include "Graphics/RenderGraph/IRenderPass.h"
#include "Graphics/RenderGraph/ResourceHandle.h"

#include "Graphics/API/ShaderResourceBlueprint.h"
#include "Graphics/API/ShaderResourceGroup.h"

namespace Mule
{
	class SolidGeometryPass : public RenderGraph::IRenderPass
	{
	public:
		SolidGeometryPass(Ref<ServiceManager> serviceManager, WeakRef<RenderGraph::RenderGraph> graph);

		bool Validate() override;
		void Setup() override;
		void Render(Ref<CommandBuffer> cmd, WeakRef<Scene> scene) override;
		void Resize(uint32_t width, uint32_t height) override;

		// Inputs
		void SetCameraBufferHandle(RenderGraph::ResourceHandle<UniformBuffer> bufferHandle);
		void SetFramebufferHandle(RenderGraph::ResourceHandle<Framebuffer> framebufferHandle);
	private:
		Ref<ServiceManager> mServiceManager;

		// Inputs
		RenderGraph::ResourceHandle<UniformBuffer> mCameraBufferHandle;
		RenderGraph::ResourceHandle<Framebuffer> mFramebufferHandle;

		// Pass Generated
		RenderGraph::ResourceHandle<ShaderResourceGroup> mShaderResourceGroupHandle;
		Ref<ShaderResourceBlueprint> mShaderResourceBlueprint;

		
	};
}