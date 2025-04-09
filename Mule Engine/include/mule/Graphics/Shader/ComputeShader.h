#pragma once

#include "Asset/Asset.h"
#include "Graphics/DescriptorSetLayout.h"

#include <Volk/volk.h>

namespace Mule
{
	class GraphicsContext;

	struct ComputeShaderDescription
	{
		fs::path Filepath;
		std::vector<WeakRef<DescriptorSetLayout>> Layouts;
		uint32_t PushConstantSize = 0;
	};

	class ComputeShader : public Asset<AssetType::Shader>
	{
	public:
		ComputeShader(WeakRef<GraphicsContext> context, const ComputeShaderDescription& description);
		virtual ~ComputeShader();

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	private:
		VkDevice mDevice;
		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;

		VkShaderModule CreateComputeModule(const fs::path& filepath);
	};
}
