#include "Graphics/API/ComputePipeline.h"

#include "Graphics/API/GraphicsContext.h"

// API
#include "Graphics/API/Vulkan/Pipeline/VulkanComputePipeline.h"

namespace Mule
{
	Ref<ComputePipeline> ComputePipeline::Create(const ComputePipelineDescription& description)
	{
		GraphicsAPI API = GraphicsContext::Get().GetAPI();

		switch (API)
		{
		case Mule::GraphicsAPI::Vulkan: return MakeRef<Vulkan::VulkanComputePipeline>(description);
		case Mule::GraphicsAPI::None:
		default:
			return nullptr;
		}
	}
}