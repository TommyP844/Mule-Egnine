
#include "Graphics/ComputeShader.h"

#include "Graphics/Context/GraphicsContext.h"

#include <spdlog/spdlog.h>
#include <shaderc/shaderc.h>

#include <fstream>
#include <vector>

namespace Mule
{
	ComputeShader::ComputeShader(WeakRef<GraphicsContext> context, const ComputeShaderDescription& description)
		:
		Asset(description.Filepath),
		mDevice(context->GetDevice())
	{
		VkShaderModule module = CreateComputeModule(description.Filepath);
		
		std::vector<VkDescriptorSetLayout> layouts;
		for (auto descriptorSetLayout : description.Layouts)
		{
			layouts.push_back(descriptorSetLayout->GetLayout());
		}

		
		VkPushConstantRange range{};

		range.offset = 0;
		range.size = description.PushConstantSize;
		range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		// Create pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = description.PushConstantSize > 0 ? 1 : 0;
		pipelineLayoutInfo.pPushConstantRanges = description.PushConstantSize > 0 ? &range : nullptr;
		
		if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create compute pipeline layout for {}", description.Filepath.string());
		}

		VkPipelineShaderStageCreateInfo computeShaderStage{};
		computeShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStage.module = module;
		computeShaderStage.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo{};

		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = computeShaderStage;
		pipelineInfo.layout = mPipelineLayout;


		VkResult result = vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("failed to create compute pipeline: {}", description.Filepath.string());
		}

		if (module)
		{
			vkDestroyShaderModule(mDevice, module, nullptr);
		}
	}

	ComputeShader::~ComputeShader()
	{
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
		vkDestroyPipeline(mDevice, mPipeline, nullptr);
	}

	VkShaderModule ComputeShader::CreateComputeModule(const fs::path& filepath)
	{
		std::fstream file(filepath);
		if (!file.is_open())
		{
			return VK_NULL_HANDLE;
		}

		std::string code, line;
		while (std::getline(file, line))
		{
			code += line + "\n";
		}
		file.close();

		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();

		shaderc_compilation_result_t compileResult = shaderc_compile_into_spv(compiler, code.data(), code.size(), shaderc_shader_kind::shaderc_compute_shader, filepath.string().c_str(), "main", options);

		shaderc_compilation_status compileStatus = shaderc_result_get_compilation_status(compileResult);
		if (compileStatus != shaderc_compilation_status::shaderc_compilation_status_success)
		{
			const char* errorMsg = shaderc_result_get_error_message(compileResult);
			SPDLOG_ERROR(errorMsg);
			return VK_NULL_HANDLE;
		}

		size_t size = shaderc_result_get_length(compileResult);
		std::vector<uint32_t> compiledBytes(size);
		uint32_t* data = (uint32_t*)shaderc_result_get_bytes(compileResult);
		memcpy(compiledBytes.data(), data, size);
		shaderc_result_release(compileResult);
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);

		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = compiledBytes.size();
		info.pCode = compiledBytes.data();
		info.pNext = nullptr;
		info.flags = 0;

		VkShaderModule module = VK_NULL_HANDLE;
		VkResult result = vkCreateShaderModule(mDevice, &info, nullptr, &module);
		if (result != VK_SUCCESS)
		{
			SPDLOG_ERROR("Failed to create shader module for shader: {}", filepath.string());
			return VK_NULL_HANDLE;
		}

		return module;
	}
}