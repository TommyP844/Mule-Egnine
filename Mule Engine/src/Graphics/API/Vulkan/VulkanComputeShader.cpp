
#include "Graphics/API/Vulkan/VulkanComputeShader.h"

#include "Graphics/API/Vulkan/VulkanGraphicsContext.h"

#include <spdlog/spdlog.h>
#include <shaderc/shaderc.h>

#include <fstream>
#include <vector>

namespace Mule
{
	VulkanComputeShader::VulkanComputeShader(WeakRef<GraphicsContext> context, const fs::path& filepath)
		:
		Asset(filepath),
		IVulkanShader(context->GetDevice())
	{
		std::vector<VkPipelineShaderStageCreateInfo> info{};
		bool success = Load(filepath, info, {});
		
		for (auto& [set, desc] : mOptions.DescriptorSetLayoutDescriptions) 
		{
			std::vector<LayoutDescription> layouts;
			for (auto& [binding, layoutDesc] : desc)
			{
				layouts.push_back(layoutDesc);
			}
			mOptions.DescriptorLayouts.push_back(context->CreateDescriptorSetLayout(layouts));
		}

		std::vector<VkDescriptorSetLayout> layouts;
		for (const auto& layout : mOptions.DescriptorLayouts)
		{
			layouts.push_back(layout->GetLayout());
		}

		uint32_t PCSize = 0;
		for (auto& range : mOptions.PushConstants)
		{
			PCSize += range.Size;
		}
		
		VkPushConstantRange range{};

		range.offset = 0;
		range.size = PCSize;
		range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		// Create pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = range.size > 0 ? 1 : 0;
		pipelineLayoutInfo.pPushConstantRanges = &range;
		
		if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create compute pipeline layout for {}", filepath.string());
		}

		VkPipelineShaderStageCreateInfo computeShaderStage{};
		computeShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStage.module = info[0].module;
		computeShaderStage.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo{};

		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = computeShaderStage;
		pipelineInfo.layout = mPipelineLayout;


		VkResult result = vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to create compute pipeline: {}", filepath.string());
		}

		if (info[0].module)
		{
			vkDestroyShaderModule(mDevice, info[0].module, nullptr);
		}
	}

	VulkanComputeShader::~VulkanComputeShader()
	{
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
		vkDestroyPipeline(mDevice, mPipeline, nullptr);
	}
}