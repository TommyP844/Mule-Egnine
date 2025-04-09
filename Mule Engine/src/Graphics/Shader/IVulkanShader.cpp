
#include "Graphics/Shader/IVulkanShader.h"

#include <spdlog/spdlog.h>
#include <shaderc/shaderc.h>

#include <fstream>

namespace Mule
{
	IVulkanShader::IVulkanShader(VkDevice device)
		:
		mDevice(device),
		mPipeline(VK_NULL_HANDLE),
		mPipelineLayout(VK_NULL_HANDLE)
	{
	}

	std::map<ShaderStage, std::string> IVulkanShader::Parse(const fs::path& sourcePath)
	{
		std::fstream file(sourcePath);
		if (!file.is_open())
			return {};

		ShaderStage currentStage = ShaderStage::None;
		std::string line;
		std::map<ShaderStage, std::string> sources;
		std::map<ShaderStage, std::vector<uint32_t>> codes;

		uint32_t lineCount = 0;
		while (std::getline(file, line))
		{
			lineCount++;
			if (line == "#SETUP")
			{
				currentStage = ShaderStage::Setup;
				continue;
			}
			else if (line == "#VERTEX")
			{
				currentStage = ShaderStage::Vertex;
				continue;
			}
			else if (line == "#FRAGMENT")
			{
				currentStage = ShaderStage::Fragment;
				continue;
			}
			else if (line == "#GEOMETRY")
			{
				currentStage = ShaderStage::Geometry;
				continue;
			}
			else if (line == "#TESSELATION-CONTOL")
			{
				currentStage = ShaderStage::TesselationControl;
				continue;
			}
			else if (line == "#TESSELATION-EVALUATION")
			{
				currentStage = ShaderStage::TesselationEvaluation;
				continue;
			}

			if (currentStage == ShaderStage::None)
				continue;

			sources[currentStage] += line + "\n";
		}
		file.close();

		return sources;
	}
	
	VkShaderModule IVulkanShader::CreateShaderModule(const std::vector<uint32_t>& source)
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
	
	std::vector<uint32_t> IVulkanShader::Compile(ShaderStage stage, const std::string& source, const fs::path& filepath, const std::vector<std::pair<std::string, std::string>>& macros)
	{
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t compileOptions = shaderc_compile_options_initialize();

		for (const auto& [name, value] : macros)
		{
			shaderc_compile_options_add_macro_definition(compileOptions, name.c_str(), name.size(), value.c_str(), value.size());
		}


		shaderc_shader_kind kind = shaderc_fragment_shader;
		switch (stage)
		{
		case ShaderStage::Vertex: kind = shaderc_shader_kind::shaderc_vertex_shader; break;
		case ShaderStage::Fragment: kind = shaderc_shader_kind::shaderc_fragment_shader; break;
		case ShaderStage::Geometry: kind = shaderc_shader_kind::shaderc_geometry_shader; break;
		case ShaderStage::TesselationControl: kind = shaderc_shader_kind::shaderc_glsl_tess_control_shader; break;
		case ShaderStage::TesselationEvaluation: kind = shaderc_shader_kind::shaderc_tess_evaluation_shader; break;
		default:
			SPDLOG_WARN("Failed to deduce shader type: {} - {}", (uint32_t)stage, filepath.string());
			shaderc_compile_options_release(compileOptions);
			shaderc_compiler_release(compiler);
			return {};
		}
		shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, source.c_str(), source.size(), kind, filepath.string().c_str(), "main", compileOptions);

		shaderc_compilation_status compileStatus = shaderc_result_get_compilation_status(result);
		if (compileStatus != shaderc_compilation_status::shaderc_compilation_status_success)
		{
			const char* errorMsg = shaderc_result_get_error_message(result);
			SPDLOG_ERROR(errorMsg);
			shaderc_compile_options_release(compileOptions);
			shaderc_compiler_release(compiler);
			return {};
		}

		size_t size = shaderc_result_get_length(result);
		std::vector<uint32_t> compiledBytes(size);
		uint32_t* data = (uint32_t*)shaderc_result_get_bytes(result);
		memcpy(compiledBytes.data(), data, size);
		shaderc_result_release(result);

		shaderc_compile_options_release(compileOptions);
		shaderc_compiler_release(compiler);

		return compiledBytes;
	}

	VkPipelineShaderStageCreateInfo IVulkanShader::CreateStage(const std::vector<uint32_t>& source, ShaderStage stage)
	{
		VkShaderStageFlagBits vkStage{};
		switch (stage)
		{
		case Mule::ShaderStage::Vertex: vkStage = VK_SHADER_STAGE_VERTEX_BIT; break;
		case Mule::ShaderStage::Fragment: vkStage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
		case Mule::ShaderStage::Geometry: vkStage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
		case Mule::ShaderStage::TesselationControl: vkStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
		case Mule::ShaderStage::TesselationEvaluation: vkStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
		case Mule::ShaderStage::Compute: vkStage = VK_SHADER_STAGE_COMPUTE_BIT; break;
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
}