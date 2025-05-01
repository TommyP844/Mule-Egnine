
#include "Graphics/API/Vulkan/Pipeline/VulkanComputePipeline.h"

#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/VulkanDescriptorSetLayout.h"

#include <spdlog/spdlog.h>
#include <shaderc/shaderc.h>

#include <fstream>
#include <vector>

namespace Mule::Vulkan
{
	VulkanComputePipeline::VulkanComputePipeline(const ComputePipelineDescription& description)
		:
		mPipeline(VK_NULL_HANDLE),
		mPipelineLayout(VK_NULL_HANDLE)
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		std::map<ShaderStage, std::vector<uint32_t>> byteCodes;
		
		Compile(description.Filepath, byteCodes, {});
		ReflectionData reflectionData = Reflect(byteCodes);
		auto stages = BuildShaderStages(byteCodes);

		std::vector<VkDescriptorSetLayout> layouts;
		for (auto layout : reflectionData.Layouts)
		{
			mBlueprints.push_back(layout);
			Ref<VulkanDescriptorSetLayout> vkLayout = layout;
			layouts.push_back(vkLayout->GetLayout());
		}

		uint32_t pcSize = 0;

		for (auto pc : reflectionData.PushConstants)
		{
			pcSize = pc.Size;
			break;
		}

		VkPushConstantRange range{
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
			.offset = 0,
			.size = pcSize,
		};

		// Create pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = static_cast<uint32_t>(layouts.size()),
			.pSetLayouts = layouts.data(),
			.pushConstantRangeCount = (range.size > 0) ? 1u : 0u,
			.pPushConstantRanges = &range,
		};
		
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create compute pipeline layout for {}", description.Filepath.string());
		}
				
		VkComputePipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = stages[0],
			.layout = mPipelineLayout,
		};

		VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline);

		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to create compute pipeline: {}", description.Filepath.string());
		}

		if (stages[0].module)
		{
			vkDestroyShaderModule(device, stages[0].module, nullptr);
		}
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
		vkDestroyPipeline(device, mPipeline, nullptr);
	}
}