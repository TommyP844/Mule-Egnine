#pragma once

#include "Asset/Asset.h"
#include "VulkanDescriptorSetLayout.h"
#include "IVulkanShader.h"

#include <Volk/volk.h>

namespace Mule
{
	class VulkanComputeShader : public Asset<AssetType::ComputeShader>, public IVulkanShader
	{
	public:
		VulkanComputeShader(const fs::path& filepath);
		virtual ~VulkanComputeShader();

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	private:
	};
}
