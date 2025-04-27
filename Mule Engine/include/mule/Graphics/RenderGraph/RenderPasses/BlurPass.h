#pragma once

#include "Graphics/RenderGraph/IRenderPass.h"

namespace Mule
{
	class BlurPass : RenderGraph::IRenderPass
	{
	public:
		BlurPass(WeakRef<RenderGraph::RenderGraph> graph);
		virtual ~BlurPass() = default;

		bool Validate() override;
		void Setup() override;
		void Render(Ref<CommandBuffer> cmd, WeakRef<Scene> scene) override;

	private:
		
	};
}