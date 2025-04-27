#include "Graphics/RenderGraph/RenderPasses/BlurPass.h"

#include "Graphics/API/ShaderResourceGroup.h"


namespace Mule
{
	BlurPass::BlurPass(WeakRef<RenderGraph::RenderGraph> graph)
		:
		IRenderPass("BlurPass", graph)
	{
	}

	void BlurPass::Render(Ref<CommandBuffer> cmd, WeakRef<Scene> scene)
	{
		
	}

	bool BlurPass::Validate()
	{
		return true;
	}

	// this will be called twice by the render graph, once for each frame in flight so both resources get updated
	void BlurPass::Setup()
	{
		
	}
}