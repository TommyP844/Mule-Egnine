#pragma once

#include "Asset/Asset.h"
#include "Graphics/DescriptorSetLayout.h"
#include "IVulkanShader.h"

#include <Volk/volk.h>

namespace Mule
{
	class GraphicsContext;

	class ComputeShader : public Asset<AssetType::ComputeShader>, public IVulkanShader
	{
	public:
		ComputeShader(WeakRef<GraphicsContext> context, const fs::path& filepath);
		virtual ~ComputeShader();

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	private:
	};
}
