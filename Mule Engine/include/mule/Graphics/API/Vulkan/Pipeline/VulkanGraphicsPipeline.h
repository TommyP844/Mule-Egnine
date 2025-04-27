#pragma once

// Engine
#include "Graphics/API/GraphicsPipeline.h"
#include "Graphics/API/Vulkan/Pipeline/IVulkanPipeline.h"

#include <Volk/volk.h>


namespace fs = std::filesystem;

namespace Mule::Vulkan
{

	class VulkanGraphicsPipeline : public GraphicsPipeline, public IVulkanPipeline
	{
	public:
		VulkanGraphicsPipeline(const GraphicsPipelineDescription& description);
		~VulkanGraphicsPipeline();

		void Reload() override;

		const PushConstantInfo& GetPushConstant(VkShaderStageFlags stage) const;

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	private:
		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;

		GraphicsPipelineDescription mDescription;

		std::unordered_map<VkShaderStageFlags, PushConstantInfo> mPushConstants;
	};
}
