#pragma once

#include "Ref.h"
#include "Graphics/Renderer/RenderGraph/RenderPasses/RenderPass.h"
#include "Graphics/Renderer/RenderGraph/ResourceHandle.h"
#include "Graphics/Renderer/RenderGraph/ResourceBuilder.h"

namespace Mule
{
	class GBufferPass : public RenderPass
	{
	public:
		GBufferPass(ResourceBuilder& resourceBuilder);

		void Setup(ResourceRegistry& registry) override;
		void Render(Ref<CommandBuffer> cmd, const std::vector<RenderCommand>& commands, const ResourceRegistry& registry) override;
		void Resize(ResourceRegistry& registry, uint32_t width, uint32_t height) override;

	private:
		ResourceHandle mFramebufferHandle;
		ResourceHandle mCameraSRGHandle;
		ResourceHandle mBindlessTextureSRGHandle;
		ResourceHandle mBindlessMaterialSRGHandle;
		ResourceHandle mCameraBufferHandle;
	};
}