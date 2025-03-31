#include "Graphics/GraphicsShader.h"

// Submodules
#include <spdlog/spdlog.h>
#include <shaderc/shaderc.h>

// STD
#include <vector>
#include <fstream>
#include <map>


namespace Mule
{
	GraphicsShader::GraphicsShader(VkDevice device, const GraphicsShaderDescription& description)
		:
		Asset(GenerateUUID(), description.SourcePath),
		mIsValid(false),
		mDevice(device),
		mPipeline(VK_NULL_HANDLE),
		mPipelineLayout(VK_NULL_HANDLE),
		mDescription(description)
	{
		Reload();
	}

	GraphicsShader::~GraphicsShader()
	{
		if(mPipelineLayout)
			vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);

		if(mPipeline)
			vkDestroyPipeline(mDevice, mPipeline, nullptr);
	}

	void GraphicsShader::Reload()
	{
		mIsValid = false;

		vkDeviceWaitIdle(mDevice);

		if (mPipelineLayout)
		{
			vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
			mPipelineLayout = VK_NULL_HANDLE;
		}

		if (mPipeline)
		{
			vkDestroyPipeline(mDevice, mPipeline, nullptr);
			mPipeline = VK_NULL_HANDLE;
		}

		std::vector<VkPipelineShaderStageCreateInfo> stages;
		bool compileResult = Compile(mDescription.SourcePath, stages, mDescription.Macros);
		if (!compileResult)
		{
			return;
		}

		// Vertex Input State
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

		// Vertex Input Assembly State
		VkPipelineInputAssemblyStateCreateInfo vertexInputAssemblyState{};
		{
			vertexInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			vertexInputAssemblyState.pNext = nullptr;
			vertexInputAssemblyState.flags = 0;
			vertexInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			vertexInputAssemblyState.primitiveRestartEnable = VK_FALSE;
		}

		// Viewport State
		VkPipelineViewportStateCreateInfo viewportState{};
		{
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.pNext = nullptr;
			viewportState.flags = 0;
			viewportState.viewportCount = 1;
			viewportState.pViewports = nullptr;
			viewportState.scissorCount = 1;
			viewportState.pScissors = nullptr;
		}

		// Rasterizor State
		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		{
			rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationState.pNext = nullptr;
			rasterizationState.flags = 0;
			rasterizationState.depthClampEnable;
			rasterizationState.rasterizerDiscardEnable;
			rasterizationState.polygonMode = (VkPolygonMode)mDescription.FillMode;
			rasterizationState.cullMode = (VkCullModeFlagBits)mDescription.CulleMode;
			rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizationState.depthBiasEnable = VK_FALSE;
			rasterizationState.depthBiasConstantFactor;
			rasterizationState.depthBiasClamp;
			rasterizationState.depthBiasSlopeFactor;
			rasterizationState.lineWidth = mDescription.LineWidth;
		}

		// MultiSample state
		VkPipelineMultisampleStateCreateInfo multiSampleState{};
		{
			multiSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multiSampleState.pNext = nullptr;
			multiSampleState.flags = 0;
			multiSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multiSampleState.sampleShadingEnable = VK_FALSE;
			// multiSampleState.minSampleShading; Unused
			// multiSampleState.pSampleMask; Unused
			multiSampleState.alphaToCoverageEnable = VK_FALSE;
			// multiSampleState.alphaToOneEnable; Unused
		}

		// Depth/Stencil State
		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		{
			depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilState.pNext = nullptr;
			depthStencilState.flags = 0;
			depthStencilState.depthTestEnable = mDescription.EnableDepthTesting;
			depthStencilState.depthWriteEnable = mDescription.WriteDepth;
			depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencilState.stencilTestEnable = VK_FALSE;
			depthStencilState.front = {};
			depthStencilState.back = {};
			depthStencilState.depthBoundsTestEnable = VK_TRUE;
			depthStencilState.minDepthBounds = 0.f;
			depthStencilState.maxDepthBounds = 1.f;
		}

		// Color Blend State
		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
		{
			const auto& attachments = mDescription.RenderPass->GetColorAttachments();
			for (auto attachment : attachments)
			{
				VkPipelineColorBlendAttachmentState attachmentState{};

				attachmentState.blendEnable = attachment.BlendEnable & mDescription.BlendEnable;
				attachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				attachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				attachmentState.colorBlendOp = VK_BLEND_OP_ADD;
				attachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				attachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				attachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
				attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

				colorBlendAttachments.push_back(attachmentState);
			}

			colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendState.pNext = nullptr;
			colorBlendState.flags = 0;
			colorBlendState.logicOpEnable = VK_FALSE;
			colorBlendState.logicOp = VK_LOGIC_OP_NO_OP;
			colorBlendState.attachmentCount = colorBlendAttachments.size();
			colorBlendState.pAttachments = colorBlendAttachments.data();
			// colorBlendState.blendConstants[4];
		}


		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		// Dynamic States
		VkPipelineDynamicStateCreateInfo dynamicState{};
		{
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.pNext = nullptr;
			dynamicState.flags = 0;
			dynamicState.dynamicStateCount = dynamicStates.size();
			dynamicState.pDynamicStates = dynamicStates.data();
		}

		// Pipeline Layout
		std::vector<VkPushConstantRange> pushConstantRanges{};
		std::vector<VkDescriptorSetLayout> layouts;
		{
			for (const auto& layout : mDescription.DescriptorLayouts)
			{
				layouts.push_back(layout->GetLayout());
			}

			uint32_t offset = 0;
			for (auto& range : mDescription.PushConstants)
			{
				VkPushConstantRange pcr{};

				pcr.stageFlags = (VkShaderStageFlagBits)range.Stage;
				pcr.size = range.Size;
				pcr.offset = offset;

				pushConstantRanges.push_back(pcr);
				mPushConstantMapping[range.Stage] = { offset, range.Size };

				offset += pcr.size;
			}

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};

			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.pNext = nullptr;
			pipelineLayoutCreateInfo.flags = 0;
			pipelineLayoutCreateInfo.setLayoutCount = layouts.size();
			pipelineLayoutCreateInfo.pSetLayouts = layouts.data();
			pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();
			pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

			VkResult result = vkCreatePipelineLayout(mDevice, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout);
			if (result != VK_SUCCESS)
			{
				mIsValid = false;
				SPDLOG_ERROR("Failed to create pipeline layout for shader: {}", mDescription.SourcePath.string());
			}
		}

		VkGraphicsPipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stageCount = stages.size();
		createInfo.pStages = stages.data();
		createInfo.pVertexInputState = &vertexInputState;
		createInfo.pInputAssemblyState = &vertexInputAssemblyState;
		createInfo.pTessellationState = nullptr;
		createInfo.pViewportState = &viewportState;
		createInfo.pRasterizationState = &rasterizationState;
		createInfo.pMultisampleState = &multiSampleState;
		createInfo.pDepthStencilState = &depthStencilState;
		createInfo.pColorBlendState = &colorBlendState;
		createInfo.pDynamicState = &dynamicState;
		createInfo.layout = mPipelineLayout;
		createInfo.renderPass = mDescription.RenderPass->GetHandle();
		createInfo.subpass = mDescription.Subpass;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = 0;

		VkResult result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline);
		if (result != VK_SUCCESS)
		{
			mIsValid = false;
			SPDLOG_ERROR("Failed to load shader: {}", mDescription.SourcePath.string());
		}
		else
		{
			mIsValid = true;
		}

		for (auto stage : stages)
		{
			if (stage.module)
			{
				vkDestroyShaderModule(mDevice, stage.module, nullptr);
			}
		}
	}

	const std::pair<uint32_t, uint32_t>& GraphicsShader::GetPushConstantRange(ShaderStage stage)
	{
		auto iter = mPushConstantMapping.find(stage);
		if (iter == mPushConstantMapping.end())
		{
			SPDLOG_WARN("Failed to find push constant for shader: {}, and stage: {}", Name(), (uint32_t)stage);
			return {0, 0};
		}
		return iter->second;
	}

	bool GraphicsShader::Compile(const fs::path& sourcePath, std::vector<VkPipelineShaderStageCreateInfo>& stages, const std::vector<std::pair<std::string, std::string>>& macros)
	{
		std::fstream file(sourcePath);
		if (!file.is_open())
			return false;

		std::map<ShaderStage, std::string> sources;
		std::map<ShaderStage, std::pair<uint32_t, uint32_t>> shaderLines;

		ShaderStage currentStage = ShaderStage::None;
		std::string line;

		uint32_t lineCount = 0;
		while (std::getline(file, line))
		{
			lineCount++;

			if (line == "#VERTEX") 
			{
				currentStage = ShaderStage::Vertex; 
				shaderLines[currentStage] = { lineCount, 0 };
				continue; 
			}
			if (line == "#FRAGMENT")
			{ 
				currentStage = ShaderStage::Fragment;
				shaderLines[currentStage] = { lineCount, 0 };
				continue; 
			}
			if (line == "#GEOMETRY") 
			{
				currentStage = ShaderStage::Geometry;
				shaderLines[currentStage] = { lineCount, 0 };
				continue; 
			}
			if (line == "#TESSELATION-CONTOL") 
			{ 
				currentStage = ShaderStage::TesselationControl;
				shaderLines[currentStage] = { lineCount, 0 };
				continue;
			}
			if (line == "#TESSELATION-EVALUATION")
			{ 
				currentStage = ShaderStage::TesselationEvaluation;
				shaderLines[currentStage] = { lineCount, 0 };
				continue; 
			}

			if (currentStage == ShaderStage::None)
				continue;

			shaderLines[currentStage].second++;

			sources[currentStage] += line + "\n";
		}
		file.close();

		bool success = true;
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t compileOptions = shaderc_compile_options_initialize();

		for (const auto& [name, value] : macros)
		{
			shaderc_compile_options_add_macro_definition(compileOptions, name.c_str(), name.size(), value.c_str(), value.size());
		}

		for (auto [stage, source] : sources)
		{
			shaderc_shader_kind kind = shaderc_fragment_shader;
			switch (stage)
			{
			case ShaderStage::Vertex: kind = shaderc_shader_kind::shaderc_vertex_shader; break;
			case ShaderStage::Fragment: kind = shaderc_shader_kind::shaderc_fragment_shader; break;
			case ShaderStage::Geometry: kind = shaderc_shader_kind::shaderc_geometry_shader; break;
			case ShaderStage::TesselationControl: kind = shaderc_shader_kind::shaderc_glsl_tess_control_shader; break;
			case ShaderStage::TesselationEvaluation: kind = shaderc_shader_kind::shaderc_tess_evaluation_shader; break;
			default:
				SPDLOG_WARN("Failed to deduce shader type: {}", sourcePath.string());
				success = false;
				break;
			}
			shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, source.c_str(), source.size(), kind, sourcePath.string().c_str(), "main", compileOptions);

			shaderc_compilation_status compileStatus = shaderc_result_get_compilation_status(result);
			if (compileStatus != shaderc_compilation_status::shaderc_compilation_status_success) 
			{
				const char* errorMsg = shaderc_result_get_error_message(result);
				SPDLOG_ERROR(errorMsg);
				success = false;
				break;
			}

			size_t size = shaderc_result_get_length(result);
			std::vector<uint32_t> compiledBytes(size);
			uint32_t* data = (uint32_t*)shaderc_result_get_bytes(result);
			memcpy(compiledBytes.data(), data, size);
			shaderc_result_release(result);

			VkPipelineShaderStageCreateInfo info = CreateStage(compiledBytes, stage);
			stages.push_back(info);
		}
		shaderc_compile_options_release(compileOptions);
		shaderc_compiler_release(compiler);

		return success;
	}

	VkPipelineShaderStageCreateInfo GraphicsShader::CreateStage(const std::vector<uint32_t>& source, ShaderStage stage)
	{
		VkShaderStageFlagBits vkStage{};
		switch (stage)
		{
		case Mule::ShaderStage::Vertex: vkStage = VK_SHADER_STAGE_VERTEX_BIT; break;
		case Mule::ShaderStage::Fragment: vkStage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
		case Mule::ShaderStage::Geometry: vkStage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
		case Mule::ShaderStage::TesselationControl: vkStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
		case Mule::ShaderStage::TesselationEvaluation: vkStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
		default:
			break;
		}

		VkPipelineShaderStageCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.stage = vkStage;
		info.module = CreateShaderModule(source);
		info.pName = "main";
		info.pSpecializationInfo = nullptr;

		return info;
	}

	VkShaderModule GraphicsShader::CreateShaderModule(const std::vector<uint32_t>& source)
	{
		VkShaderModuleCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.codeSize = source.size();
		info.pCode = source.data();

		VkShaderModule module;
		VkResult result = vkCreateShaderModule(mDevice, &info, nullptr, &module);
		return module;
	}
}