#pragma once

#include "Asset/Asset.h"
#include "Graphics/API/ComputePipeline.h"
#include "Graphics/API/Vulkan/Pipeline/IVulkanPipeline.h"

#include <Volk/volk.h>

namespace Mule::Vulkan
{
	class VulkanComputePipeline : public Asset<AssetType::ComputeShader>, ComputePipeline, public IVulkanPipeline
	{
	public:
		VulkanComputePipeline(const ComputePipelineDescription& description);
		virtual ~VulkanComputePipeline();

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	private:
		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;
	};
}
