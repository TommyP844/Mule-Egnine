#include "Graphics/Renderer/RenderGraph/RenderPasses/EnvironmentMapPass.h"

#include "ECS/Components.h"

#include "Graphics/EnvironmentMap.h"
#include "Asset/AssetManager.h"
#include "Graphics/ShaderFactory.h"
#include "Engine Context/EngineAssets.h"

namespace Mule
{
	EnvironmentMapPass::EnvironmentMapPass()
		:
		RenderPass("Environment Map Pass", PassType::Graphics)
	{
	}

	void EnvironmentMapPass::Setup(ResourceRegistry& registry)
	{
	}

	void EnvironmentMapPass::Render(Ref<CommandBuffer> cmd, const std::vector<RenderCommand>& commands, const ResourceRegistry& context)
	{

	}

	void EnvironmentMapPass::Resize(ResourceRegistry& context, uint32_t width, uint32_t height)
	{
	}
}