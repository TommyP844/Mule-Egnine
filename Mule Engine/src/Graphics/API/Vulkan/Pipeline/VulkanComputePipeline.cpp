
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
		mBlueprints = description.Resources;

		VkDevice device = VulkanContext::Get().GetDevice();

		VkPipelineShaderStageCreateInfo stageInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			.module = nullptr,
			.pName = "main",
			.pSpecializationInfo = nullptr,
		};		

		std::vector<VkDescriptorSetLayout> layouts;
		for (auto blueprint : description.Resources)
		{
			Ref<VulkanDescriptorSetLayout> layout = blueprint;
			layouts.push_back(layout->GetLayout());
		}
		
		VkPushConstantRange range{
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
			.offset = 0,
			.size = description.ConstantBufferSize,
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

		VkPipelineShaderStageCreateInfo computeShaderStage{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			.module = stageInfo.module,
			.pName = "main",
		};

		VkComputePipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = computeShaderStage,
			.layout = mPipelineLayout,
		};

		VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to create compute pipeline: {}", description.Filepath.string());
		}

		if (stageInfo.module)
		{
			vkDestroyShaderModule(device, stageInfo.module, nullptr);
		}
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
		vkDestroyPipeline(device, mPipeline, nullptr);
	}
}