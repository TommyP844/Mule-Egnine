#pragma once

#include "Asset/Asset.h"
#include "Graphics/Context/GraphicsContext.h"
#include "DescriptorSetLayout.h"

#include <vulkan/vulkan.h>

namespace Mule
{
	struct ComputeShaderDescription
	{
		fs::path Filepath;
		std::vector<WeakRef<DescriptorSetLayout>> Layouts;
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
