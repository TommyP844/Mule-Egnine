#pragma once

#include "Graphics/Renderer/RenderGraph/RenderPasses/RenderPass.h"

namespace Mule
{
	class EnvironmentMapPass : public RenderPass
	{
	public:
		EnvironmentMapPass();

		void Setup(ResourceRegistry& registry) override;
		void Render(Ref<CommandBuffer> cmd, const std::vector<RenderCommand>& commands, const ResourceRegistry& context) override;
		void Resize(ResourceRegistry& context, uint32_t width, uint32_t height) override;

	private:
		
	};
}