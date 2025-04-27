
#include "Graphics/API/GraphicsPipeline.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Pipeline/VulkanGraphicsPipeline.h"

namespace Mule
{
	Ref<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineDescription& description)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanGraphicsPipeline>(description);
		case Mule::GraphicsAPI::None:
		default: return nullptr;
		}
	}
}