#include "Graphics/Shader/GraphicsShader.h"

#include "Graphics/Context/GraphicsContext.h"
#include "Core/StringUtil.h"

// Submodules
#include <spdlog/spdlog.h>
#include <shaderc/shaderc.h>
#include <../Source/SPIRV-Reflect/spirv_reflect.h>
#include <../Source/SPIRV-Reflect/spirv_reflect.c> // For SPIRV-Reflect & im to lazy to compile it separately


// STD
#include <vector>
#include <fstream>
#include <map>


namespace Mule
{
	GraphicsShader::GraphicsShader(WeakRef<GraphicsContext> context, const fs::path& filepath)
		:
		Asset(GenerateUUID(), filepath),
		IVulkanShader(context->GetDevice()),
		mIsValid(false),
		mContext(context)
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
		bool compileResult = Load(mFilepath, stages, {});
		if (!compileResult)
		{
			return;
		}

		// Options have been loaded
		std::sort(mOptions.Attachments.begin(), mOptions.Attachments.end(),
			[](const ShaderAttachment& a, const ShaderAttachment& b)
			{
				return a.Location < b.Location;
			});

		for (auto& [set, desc] : mOptions.DescriptorSetLayoutDescriptions) {

			DescriptorSetLayoutDescription descriptorSetLayoutDesc{};
			for (auto& [binding, layoutDesc] : desc)
			{
				descriptorSetLayoutDesc.Layouts.push_back(layoutDesc);
			}
			mOptions.DescriptorLayouts.push_back(mContext->CreateDescriptorSetLayout(descriptorSetLayoutDesc));
		}

		// Vertex Input State
		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		std::vector<VkVertexInputAttributeDescription> attributes;
		VkVertexInputBindingDescription vertexBindingDesc{};
		{
			uint32_t offset = 0;
			const std::vector<AttributeType>& attributeLayout = mOptions.VertexLayout.GetAttributes();
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
			rasterizationState.polygonMode = (VkPolygonMode)mOptions.FillMode;
			rasterizationState.cullMode = (VkCullModeFlagBits)mOptions.CulleMode;
			rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizationState.depthBiasEnable = VK_FALSE;
			rasterizationState.depthBiasConstantFactor;
			rasterizationState.depthBiasClamp;
			rasterizationState.depthBiasSlopeFactor;
			rasterizationState.lineWidth = mOptions.LineWidth;
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
			depthStencilState.depthTestEnable = mOptions.EnableDepthTesting;
			depthStencilState.depthWriteEnable = mOptions.WriteDepth;
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
			for (auto attachment : mOptions.Attachments)
			{
				VkPipelineColorBlendAttachmentState attachmentState{};
			
				attachmentState.blendEnable = attachment.BlendEnable;
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
			colorBlendState.blendConstants[4];
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
			for (const auto& layout : mOptions.DescriptorLayouts)
			{
				layouts.push_back(layout->GetLayout());
			}

			uint32_t offset = 0;
			for (auto& range : mOptions.PushConstants)
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
				SPDLOG_ERROR("Failed to create pipeline layout for shader: {}", mFilepath.string());
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
		createInfo.renderPass = VK_NULL_HANDLE;
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = 0;

		std::vector<VkFormat> attachmentFormats;
		for (auto attachment : mOptions.Attachments)
		{
			attachmentFormats.push_back((VkFormat)attachment.Format);
		}

		VkPipelineRenderingCreateInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount = attachmentFormats.size();
		renderingInfo.pColorAttachmentFormats = attachmentFormats.data();
		renderingInfo.depthAttachmentFormat = (VkFormat)mOptions.DepthFormat;
		renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
		renderingInfo.viewMask = 0;
		renderingInfo.pNext = nullptr;

		createInfo.pNext = &renderingInfo;


		VkResult result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline);
		if (result != VK_SUCCESS)
		{
			mIsValid = false;
			SPDLOG_ERROR("Failed to load shader: {}", mFilepath.string());
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

	bool GraphicsShader::Load(const fs::path& sourcePath, std::vector<VkPipelineShaderStageCreateInfo>& stages, const std::vector<std::pair<std::string, std::string>>& macros)
	{
		std::map<ShaderStage, std::string> codes = Parse(sourcePath);
		if (codes.empty())
		{
			SPDLOG_ERROR("Failed to compile shader: {}", sourcePath.string());
			return false;
		}

		std::map<ShaderStage, std::vector<uint32_t>> spirvCodes;
		for (auto [stage, code] : codes)
		{
			if (stage == ShaderStage::Setup || stage == ShaderStage::None)
				continue;

			std::vector<uint32_t> spirv = Compile(stage, code, sourcePath, macros);
			spirvCodes[stage] = spirv;
			VkPipelineShaderStageCreateInfo stageInfo = CreateStage(spirv, stage);
			stages.push_back(stageInfo);
		}
		
		ParseOptions(codes[ShaderStage::Setup]);

		for (auto [stage, code] : spirvCodes)
		{
			SpvReflectShaderModule module;
			SpvReflectResult result = spvReflectCreateShaderModule(code.size(), code.data(), &module);
			if (result != SPV_REFLECT_RESULT_SUCCESS)
			{
				SPDLOG_ERROR("Failed to get reflection data for stage: {}, in shader: {}", (uint32_t)stage, mFilepath.string());
				continue;
			}

			uint32_t var_count = 0;

			if (stage == ShaderStage::Vertex)
			{
				result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
				SpvReflectInterfaceVariable** input_vars = (SpvReflectInterfaceVariable**)malloc(var_count * sizeof(SpvReflectInterfaceVariable*));
				result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);

				std::vector<AttributeType> attributes(var_count);

				for (uint32_t i = 0; i < var_count; i++)
				{
					SpvReflectInterfaceVariable* var = input_vars[i];

					attributes[var->location] = GetAttributeTypeFromSPVFormat(var->format);

					//SPDLOG_INFO("Input Variable: {} - {} - {}", name, location, (uint32_t)var->format);
				}

				free(input_vars);

				for (auto attribute : attributes)
				{
					mOptions.VertexLayout.AddAttribute(attribute);
				}
			}

			spvReflectEnumerateDescriptorSets(&module, &var_count, NULL);
			SpvReflectDescriptorSet** sets = (SpvReflectDescriptorSet**)malloc(var_count * sizeof(SpvReflectDescriptorSet*));
			result = spvReflectEnumerateDescriptorSets(&module, &var_count, sets);

			for (uint32_t i = 0; i < var_count; i++)
			{
				SpvReflectDescriptorSet* set = sets[i];
				for (uint32_t j = 0; j < set->binding_count; j++)
				{
					SpvReflectDescriptorBinding* binding = set->bindings[j];	

					LayoutDescription desc{};
					desc.Binding = binding->binding;
					desc.ArrayCount = binding->count;
					desc.Stage = stage;
					desc.Type = GetDescriptorTypeFromSPVFormat(binding->descriptor_type);

					auto iter = mOptions.DescriptorSetLayoutDescriptions[set->set].find(binding->binding);
					if (iter != mOptions.DescriptorSetLayoutDescriptions[set->set].end())
					{
						auto s = mOptions.DescriptorSetLayoutDescriptions[set->set][binding->binding].Stage;
						s = s | stage;
						mOptions.DescriptorSetLayoutDescriptions[set->set][binding->binding].Stage = s;
					}
					else
						mOptions.DescriptorSetLayoutDescriptions[set->set][binding->binding] = desc;
				}
			}

			free(sets);

			spvReflectEnumeratePushConstants(&module, &var_count, nullptr);
			SpvReflectBlockVariable** pushConstants = (SpvReflectBlockVariable**)malloc(var_count * sizeof(SpvReflectBlockVariable*));
			result = spvReflectEnumeratePushConstants(&module, &var_count, pushConstants);

			for (uint32_t i = 0; i < var_count; i++)
			{
				SpvReflectBlockVariable* pushConstant = pushConstants[i];

				PushConstant pc{};

				pc.Stage = stage;
				pc.Size = pushConstant->size - pushConstant->offset;

				mOptions.PushConstants.push_back(pc);
			}

			free(pushConstants);

			spvReflectDestroyShaderModule(&module);
		}

		return true;
	}

	void GraphicsShader::ParseOptions(std::string options)
	{
		std::vector<std::string> lines;

		while (!options.empty())
		{
			auto split = options.find_first_of('\n');
			std::string line = options.substr(0, split);
			if (!line.empty())
			{
				lines.push_back(line);
			}
			options = options.substr(split + 1);
		}

		for (auto line : lines)
		{
			std::string option, value;

			auto delim = line.find_first_of('=');

			option = Trim(line.substr(0, delim));
			value = Trim(line.substr(delim + 1));

			LoadOption(ToLower(option), ToLower(value));
		}
	}

	void GraphicsShader::LoadOption(const std::string& option, const std::string& value)
	{
		if (option == ToLower("CullMode"))
		{
			if(value == ToLower("Front"))
				mOptions.CulleMode = CullMode::Front;
			else if (value == ToLower("Back"))
				mOptions.CulleMode = CullMode::Back;
			else if (value == ToLower("None"))
				mOptions.CulleMode = CullMode::None;
			else
				SPDLOG_ERROR("Invalid cull mode: {}", value);
		}
		else if (option == ToLower("DepthTest"))
		{
			if (value == ToLower("True"))
				mOptions.EnableDepthTesting = true;
			else if (value == ToLower("False"))
				mOptions.EnableDepthTesting = false;
			else
				SPDLOG_ERROR("Invalid depth test value: {}", value);
		}
		else if (option == ToLower("DepthWrite"))
		{
			if (value == ToLower("True"))
				mOptions.WriteDepth = true;
			else if (value == ToLower("False"))
				mOptions.WriteDepth = false;
			else
				SPDLOG_ERROR("Invalid write depth value: {}", value);
		}
		else if (option == ToLower("BlendEnable"))
		{
			if (value == ToLower("True"))
				mOptions.BlendEnable = true;
			else if (value == ToLower("False"))
				mOptions.BlendEnable = false;
			else
				SPDLOG_ERROR("Invalid blend enable value: {}", value);
		}
		else if (option == ToLower("FillMode"))
		{
			if (value == ToLower("Fill"))
				mOptions.FillMode = FillMode::Solid;
			else if (value == ToLower("Wireframe"))
				mOptions.FillMode = FillMode::Wireframe;
			else
				SPDLOG_ERROR("Invalid fill mode: {}", value);
		}
		else if (option == ToLower("LineWidth"))
		{
			mOptions.LineWidth = std::stof(value);
		}
		else if (option == ToLower("DepthFormat"))
		{
			if (value == ToLower("D32F"))
			{
				mOptions.DepthFormat = TextureFormat::D32F;
			}
		}
		else if (option == ToLower("Attachment"))
		{
			std::string v = value;
			v = RemoveChars(v, '{');
			v = RemoveChars(v, '}');
			v = RemoveChars(v, ' ');

			ShaderAttachment attachment{};
			
			std::vector<std::string> tokens = Split(v, ',');
			for (auto tokens : tokens)
			{
				std::vector<std::string> values = Split(tokens, '=');
				std::string var = values[0];
				std::string val = values[1];

				if (var == ToLower("Location"))
				{
					attachment.Location = std::stoi(val);
					mOptions.AttachmentLocations.push_back(attachment.Location);
				}
				else if (var == ToLower("Format"))
				{
					if (val == ToLower("RGBA32F"))
						attachment.Format = TextureFormat::RGBA32F;
					else if (val == ToLower("RGBA8"))
						attachment.Format = TextureFormat::RGBA8U;
					else if (val == ToLower("R32F"))
						attachment.Format = TextureFormat::R32F;
					else if (val == ToLower("R8"))
						attachment.Format = TextureFormat::R8U;
					else if (val == ToLower("RG32UI"))
						attachment.Format = TextureFormat::RG32UI;
					else if (val == ToLower("R32F"))
						attachment.Format = TextureFormat::R32F;
					else
						SPDLOG_ERROR("Invalid attachment format: {}", val);
				}
				else if (var == ToLower("BlendEnable"))
				{
					if (val == ToLower("True"))
						attachment.BlendEnable = true;
					else if (val == ToLower("False"))
						attachment.BlendEnable = false;
				}
				else
				{
					SPDLOG_ERROR("Invalid attachment option: {} - {}", var, val);
				}

			}

			mOptions.Attachments.push_back(attachment);

		}
		else
		{
			SPDLOG_ERROR("Invalid shader option: {}", option);
		}
	}

	AttributeType GraphicsShader::GetAttributeTypeFromSPVFormat(SpvReflectFormat format)
	{
		switch (format)
		{
		case SPV_REFLECT_FORMAT_R32_SFLOAT: return AttributeType::Float;
		case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return AttributeType::Vec2;
		case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return AttributeType::Vec3;
		case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return AttributeType::Vec4;
		case SPV_REFLECT_FORMAT_R32_SINT: return AttributeType::Int;
		case SPV_REFLECT_FORMAT_R32G32_SINT: return AttributeType::Int2;
		case SPV_REFLECT_FORMAT_R32G32B32_SINT: return AttributeType::Int3;
		case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: return AttributeType::Int4;

		case SPV_REFLECT_FORMAT_R32_UINT: return AttributeType::UInt;
		case SPV_REFLECT_FORMAT_R32G32_UINT: return AttributeType::UInt2;
		case SPV_REFLECT_FORMAT_R32G32B32_UINT: return AttributeType::UInt3;
		case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: return AttributeType::UInt4;
		}
		return AttributeType();
	}

	DescriptorType GraphicsShader::GetDescriptorTypeFromSPVFormat(SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			return DescriptorType::Texture;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return DescriptorType::StorageImage;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return DescriptorType::UniformBuffer;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
			assert(false && "Not implemented yet");
			break;
		default:
			break;
		}
		assert(false && "Invalid descriptor type");
		return DescriptorType();
	}
}