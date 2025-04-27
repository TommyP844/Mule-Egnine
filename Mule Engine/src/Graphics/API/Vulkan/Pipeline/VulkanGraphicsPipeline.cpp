#include "Graphics/API/Vulkan/Pipeline/VulkanGraphicsPipeline.h"
#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/VulkanTypeConversion.h"
#include "Graphics/API/Vulkan/VulkanDescriptorSetLayout.h"

// Submodules
#include <spdlog/spdlog.h>
#include <shaderc/shaderc.h>


// STD
#include <vector>
#include <fstream>
#include <map>
#include <set>

namespace Mule::Vulkan
{
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineDescription& description)
		:
		mDescription(description),
		mPipeline(VK_NULL_HANDLE),
		mPipelineLayout(VK_NULL_HANDLE)
	{
		Reload();
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
		vkDestroyPipeline(device, mPipeline, nullptr);
	}

	void VulkanGraphicsPipeline::Reload()
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		vkDeviceWaitIdle(device);

		if (mPipelineLayout)
		{
			vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
			mPipelineLayout = VK_NULL_HANDLE;
		}

		if (mPipeline)
		{
			vkDestroyPipeline(device, mPipeline, nullptr);
			mPipeline = VK_NULL_HANDLE;
		}
		
		std::map<ShaderStage, std::vector<uint32_t>> byteCodes;
		bool compileResult = Compile(mDescription.Filepath, byteCodes, {});
		if (!compileResult)
		{
			return;
		}

		IVulkanPipeline::ReflectionData data = Reflect(byteCodes);

		for (const auto& layout : data.Layouts)
		{
			mBlueprints.push_back(layout);
		}

		for (const auto& attachment : data.Attachments)
		{
			ShaderAttachment shaderAttachment{};

			shaderAttachment.Location = attachment.Location;
			shaderAttachment.Format = attachment.Format;

			mOutputAttachemnts.push_back(shaderAttachment);
		}
		

		std::vector<VkPipelineShaderStageCreateInfo> stages = BuildShaderStages(byteCodes);

		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		std::vector<VkVertexInputAttributeDescription> attributes;
		VkVertexInputBindingDescription vertexBindingDesc{};
		{
			uint32_t offset = 0;
			const std::vector<AttributeType>& attributeLayout = mDescription.VertexLayout.GetAttributes();
			for (int i = 0; i < attributeLayout.size(); i++)
			{
				auto& layoutAttribute = attributeLayout[i];

				VkVertexInputAttributeDescription attributeDescription{};
				attributeDescription.location = i;
				attributeDescription.binding = 0;
				attributeDescription.format = (VkFormat)layoutAttribute;
				attributeDescription.offset = offset;
				offset += GetAttributeSize(layoutAttribute);

				attributes.push_back(attributeDescription);
			}

			vertexBindingDesc.binding = 0;
			vertexBindingDesc.stride = offset;
			vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputState.pNext = nullptr;
			vertexInputState.flags = 0;
			vertexInputState.vertexBindingDescriptionCount = 1;
			vertexInputState.pVertexBindingDescriptions = &vertexBindingDesc;
			vertexInputState.vertexAttributeDescriptionCount = attributes.size();
			vertexInputState.pVertexAttributeDescriptions = attributes.data();
		}

		VkPipelineInputAssemblyStateCreateInfo vertexInputAssemblyState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		VkPipelineViewportStateCreateInfo viewportState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.viewportCount = 1,
			.pViewports = nullptr,
			.scissorCount = 1,
			.pScissors = nullptr
		};

		VkPipelineRasterizationStateCreateInfo rasterizationState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			// .depthClampEnable,
			// .rasterizerDiscardEnable,
			.polygonMode = GetPolygonMode(mDescription.FilleMode),
			.cullMode = GetCullMode(mDescription.CullMode),
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			// .depthBiasConstantFactor,
			// .depthBiasClamp,
			// .depthBiasSlopeFactor,
			.lineWidth = mDescription.LineWidth,
		};

		VkPipelineMultisampleStateCreateInfo multiSampleState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			// .minSampleShading,
			// .pSampleMask,
			.alphaToCoverageEnable = VK_FALSE,
			// .alphaToOneEnable,
		};

		// Depth/Stencil State
		VkPipelineDepthStencilStateCreateInfo depthStencilState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = mDescription.EnableDepthTest,
			.depthWriteEnable = mDescription.EnableDepthWrite,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_TRUE,
			.stencilTestEnable = VK_FALSE,
			.front = {},
			.back = {},
			.minDepthBounds = 0.f,
			.maxDepthBounds = 1.f
		};
		
		std::vector<VkPipelineColorBlendAttachmentState> attachmentStates(data.Attachments.size());

		for (uint32_t i = 0; i < attachmentStates.size(); i++)
		{
			VkPipelineColorBlendAttachmentState state{};

			state.blendEnable = VK_FALSE; // Blending disabled
			state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Irrelevant when blendEnable is false
			state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Irrelevant when blendEnable is false
			state.colorBlendOp = VK_BLEND_OP_ADD;             // Irrelevant when blendEnable is false
			state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Irrelevant when blendEnable is false
			state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Irrelevant when blendEnable is false
			state.alphaBlendOp = VK_BLEND_OP_ADD;             // Irrelevant when blendEnable is false
			state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;  // Write all RGBA channels

			attachmentStates[i] = state;
		}

		VkPipelineColorBlendStateCreateInfo colorBlendState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_NO_OP,
			.attachmentCount = static_cast<uint32_t>(attachmentStates.size()),
			.pAttachments = attachmentStates.data(),
			//.blendConstants[4],
		};


		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_BLEND_CONSTANTS,
			VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
			VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
			VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT
		};

		VkPipelineDynamicStateCreateInfo dynamicState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		// Push Constants
		std::set<ShaderStage> visitedStages;
		std::vector<VkPushConstantRange> pushConstantRanges{};
		{
			for (auto& pushConstant : data.PushConstants)
			{
				if (visitedStages.contains(pushConstant.Stage))
					assert("Shader stage already has a push constant");

				visitedStages.insert(pushConstant.Stage);

				VkPushConstantRange pushConstantRange {
					.stageFlags = GetShaderStage(pushConstant.Stage),
					.offset = pushConstant.Offset,
					.size = pushConstant.Size
				};

				pushConstantRanges.push_back(pushConstantRange);

				mPushConstants[GetShaderStage(pushConstant.Stage)] = pushConstant;
			}
		}

		std::vector<VkDescriptorSetLayout> layouts;
		{
			for (const auto& resource : mBlueprints)
			{
				Ref<VulkanDescriptorSetLayout> layout = resource;
				layouts.push_back(layout->GetLayout());
			}
		}
		
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.setLayoutCount = static_cast<uint32_t>(layouts.size()),
			.pSetLayouts = layouts.data(),
			.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
			.pPushConstantRanges = pushConstantRanges.data()
		};

		VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create pipeline layout for shader: {}", mDescription.Filepath.string());
		}

		std::vector<VkFormat> attachmentFormats;
		for (auto attachment : data.Attachments)
			attachmentFormats.push_back(GetVulkanFormat(attachment.Format));

		VkPipelineRenderingCreateInfo renderingInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.pNext = nullptr,
			.viewMask = 0,
			.colorAttachmentCount = static_cast<uint32_t>(attachmentFormats.size()),
			.pColorAttachmentFormats = attachmentFormats.data(),
			.depthAttachmentFormat = GetVulkanFormat(mDescription.DepthFormat),
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
		};

		VkGraphicsPipelineCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &renderingInfo,
			.flags = 0,
			.stageCount = static_cast<uint32_t>(stages.size()),
			.pStages = stages.data(),
			.pVertexInputState = &vertexInputState,
			.pInputAssemblyState = &vertexInputAssemblyState,
			.pTessellationState = nullptr,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizationState,
			.pMultisampleState = &multiSampleState,
			.pDepthStencilState = &depthStencilState,
			.pColorBlendState = &colorBlendState,
			.pDynamicState = &dynamicState,
			.layout = mPipelineLayout,
			.renderPass = VK_NULL_HANDLE,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = 0,
		};
		 
		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to load shader: {}", mDescription.Filepath.string());
		}

		for (auto stage : stages)
		{
			if (stage.module)
			{
				vkDestroyShaderModule(device, stage.module, nullptr);
			}
		}
	}

	const PushConstantInfo& VulkanGraphicsPipeline::GetPushConstant(VkShaderStageFlags stage) const
	{
		assert(mPushConstants.find(stage) != mPushConstants.end() && "Shader does not contain a push constant for stage");
		return mPushConstants.at(stage);

	}
}