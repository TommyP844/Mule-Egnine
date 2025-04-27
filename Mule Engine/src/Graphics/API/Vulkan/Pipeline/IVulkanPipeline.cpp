
#include "Graphics/API/Vulkan/Pipeline/IVulkanPipeline.h"

#include "Graphics/API/Vulkan/VulkanTypeConversion.h"
#include "Graphics/API/Vulkan/VulkanContext.h"

#include <spdlog/spdlog.h>

#include <shaderc/shaderc.h>

#include <../Source/SPIRV-Reflect/spirv_reflect.h>
#include <../Source/SPIRV-Reflect/spirv_reflect.c> // TODO: this may be an issue on other systems, should problably clone repo and setup premake

#include <fstream>

namespace Mule::Vulkan
{
	bool IVulkanPipeline::Compile(const std::filesystem::path& filepath, std::map<ShaderStage, std::vector<uint32_t>>& byteCodes, const std::vector<std::pair<std::string, std::string>>& macros)
	{
		std::ifstream file = std::ifstream(filepath);
		if (!file)
		{
			SPDLOG_ERROR("Failed to open vulkan shader file: {}", filepath.string().c_str());
			return false;
		}

		ShaderStage currentStage = ShaderStage::None;
		std::map<ShaderStage, std::string> codeLines;
		std::string line;
		while (std::getline(file, line))
		{
			if (line == "#VERTEX") { currentStage = ShaderStage::Vertex; continue; }
			if (line == "#FRAGMENT") { currentStage = ShaderStage::Fragment; continue; }
			if (line == "#GEOMETRY") { currentStage = ShaderStage::Geometry; continue; }
			if (line == "#TESS_CONTROL") { currentStage = ShaderStage::TesselationControl; continue; }
			if (line == "#TESS-EVAL") { currentStage = ShaderStage::TesselationEvaluation; continue; }
			if (line == "#COMPUTE") { currentStage = ShaderStage::Compute; continue; }

			if (currentStage == ShaderStage::None) continue;

			codeLines[currentStage] += line + "\n";
		}

		file.close();


		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();

		for (const auto& [stage, lines] : codeLines)
		{
			shaderc_shader_kind kind;
			switch (stage)
			{
			case ShaderStage::Vertex: kind = shaderc_shader_kind::shaderc_vertex_shader; break;
			case ShaderStage::Fragment: kind = shaderc_shader_kind::shaderc_fragment_shader; break;
			case ShaderStage::Geometry: kind = shaderc_shader_kind::shaderc_geometry_shader; break;
			case ShaderStage::TesselationControl: kind = shaderc_shader_kind::shaderc_tess_control_shader; break;
			case ShaderStage::TesselationEvaluation: kind = shaderc_shader_kind::shaderc_tess_evaluation_shader; break;
			case ShaderStage::Compute: kind = shaderc_shader_kind::shaderc_compute_shader; break;
			}

			bool result = CompileShaderStage(compiler, options, kind, byteCodes[stage], lines, filepath.filename().string().c_str());
			if (!result)
			{
				shaderc_compiler_release(compiler);
				shaderc_compile_options_release(options);
				return false;
			}			
		}

		shaderc_compiler_release(compiler);
		shaderc_compile_options_release(options);

		return true;
	}

	IVulkanPipeline::ReflectionData IVulkanPipeline::Reflect(std::map<ShaderStage, std::vector<uint32_t>>& byteCodes)
	{
		ReflectionData data;

		// Set -> Binding -> Description
		std::map<uint32_t, std::map<uint32_t, ShaderResourceDescription>> resourceDescriptions;

		for (const auto& [stage, bytes] : byteCodes)
		{
			// Reflect over the SPIR-V shader to get details
			SpvReflectShaderModule shaderModule;
			spvReflectCreateShaderModule(bytes.size(), bytes.data(), &shaderModule);

			// Get descriptor set layouts
			uint32_t descriptorSetCount = 0;
			spvReflectEnumerateDescriptorSets(&shaderModule, &descriptorSetCount, NULL);
			SpvReflectDescriptorSet** descriptorSets = new SpvReflectDescriptorSet*[descriptorSetCount];
			spvReflectEnumerateDescriptorSets(&shaderModule, &descriptorSetCount, descriptorSets);

			for (uint32_t i = 0; i < descriptorSetCount; ++i)
			{
				for (uint32_t j = 0; j < descriptorSets[i]->binding_count; ++j)
				{
					SpvReflectDescriptorBinding* binding = descriptorSets[i]->bindings[j];

					ShaderResourceDescription& desc = resourceDescriptions[descriptorSets[i]->set][binding->binding];
					desc.ArrayCount = binding->count;
					desc.Binding = binding->binding;
					desc.Type = GetResourceType(binding->descriptor_type);
					desc.Stages = desc.Stages | stage;
				}
			}

			// Get push constants
			uint32_t pushConstantCount = 0;
			spvReflectEnumeratePushConstants(&shaderModule, &pushConstantCount, NULL);
			SpvReflectBlockVariable** pushConstants = new SpvReflectBlockVariable*[pushConstantCount];
			spvReflectEnumeratePushConstants(&shaderModule, &pushConstantCount, pushConstants);

			for (uint32_t i = 0; i < pushConstantCount; ++i)
			{
				PushConstantInfo pushConstant{};
				pushConstant.Stage = stage;
				pushConstant.Size = pushConstants[i]->size;
				pushConstant.Offset = pushConstants[i]->offset;

				data.PushConstants.push_back(pushConstant);
			}

			if (stage == ShaderStage::Fragment)
			{
				// Get fragment shader output attachments
				uint32_t outputAttachmentCount = 0;
				spvReflectEnumerateOutputVariables(&shaderModule, &outputAttachmentCount, NULL);
				SpvReflectInterfaceVariable** outputVariables = new SpvReflectInterfaceVariable*[outputAttachmentCount];
				spvReflectEnumerateOutputVariables(&shaderModule, &outputAttachmentCount, outputVariables);

				for (uint32_t i = 0; i < outputAttachmentCount; ++i)
				{
					OutputAttachment attachment{};
					
					attachment.Location = outputVariables[i]->location;
					attachment.Format = GetTextureFormat(outputVariables[i]->format);

					data.Attachments.push_back(attachment);
				}
				
				delete[] outputVariables;
			}

			delete[] pushConstants;
			delete[] descriptorSets;

			// Clean up
			spvReflectDestroyShaderModule(&shaderModule);

		}
				
		for (const auto& [set, bindingResource] : resourceDescriptions)
		{
			std::vector<ShaderResourceDescription> resources;
			for (const auto& [binding, resource] : bindingResource)
			{
				resources.push_back(resource);
			}

			auto layout = MakeRef<VulkanDescriptorSetLayout>(resources);
			data.Layouts.push_back(layout);
		}

		return data;
	}

	std::vector<VkPipelineShaderStageCreateInfo> IVulkanPipeline::BuildShaderStages(const std::map<ShaderStage, std::vector<uint32_t>>& byteCodes)
	{
		std::vector<VkPipelineShaderStageCreateInfo> stages;

		VkDevice device = VulkanContext::Get().GetDevice();

		for (const auto& [stage, byteCode] : byteCodes)
		{
			VkShaderModule module;

			VkShaderModuleCreateInfo moduleInfo{};
			moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleInfo.pNext = nullptr;
			moduleInfo.codeSize = byteCode.size();
			moduleInfo.pCode = byteCode.data();
			moduleInfo.flags = 0;

			vkCreateShaderModule(device, &moduleInfo, nullptr, &module);

			VkPipelineShaderStageCreateInfo info{};

			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.stage = (VkShaderStageFlagBits)GetShaderStage(stage);
			info.module = module;
			info.pName = "main";
			info.pSpecializationInfo = nullptr;

			stages.push_back(info);
		}

		return stages;
	}

	bool IVulkanPipeline::CompileShaderStage(shaderc_compiler_t compiler, shaderc_compile_options_t options, shaderc_shader_kind kind, std::vector<uint32_t>& byteCode, const std::string& codeText, const char* filename)
	{
		shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, codeText.c_str(), codeText.size(), kind, filename, "main", options);

		shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
		if (status != shaderc_compilation_status::shaderc_compilation_status_success)
		{
			const char* errorMsg = shaderc_result_get_error_message(result);
			SPDLOG_ERROR("Failed to compile shader: {}, \n\t{}", filename, errorMsg);
			shaderc_result_release(result);
			return false;
		}

		const char* bytes = shaderc_result_get_bytes(result);
		size_t size = shaderc_result_get_length(result);

		byteCode.resize(size / sizeof(char));
		memcpy(byteCode.data(), bytes, size);

		shaderc_result_release(result);

		return true;
	}
	
	ShaderResourceType IVulkanPipeline::GetResourceType(SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:	return ShaderResourceType::Sampler;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:				return ShaderResourceType::StorageImage;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:			return ShaderResourceType::UniformBuffer;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
		default:
			assert(false && "Invalid descriptor type");
			break;
		}
	}
	
	TextureFormat IVulkanPipeline::GetTextureFormat(SpvReflectFormat format)
	{
		switch (format)
		{
		case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT:	return TextureFormat::RGBA_16F;
		case SPV_REFLECT_FORMAT_R32_UINT:				return TextureFormat::R_32UI;
		case SPV_REFLECT_FORMAT_R32_SINT:				return TextureFormat::RGBA_32S;
		case SPV_REFLECT_FORMAT_R32_SFLOAT:				return TextureFormat::R_32F;
		case SPV_REFLECT_FORMAT_R64_UINT:				return TextureFormat::R_64UI;
		case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:	return TextureFormat::RGBA_32F;
		case SPV_REFLECT_FORMAT_R32G32_UINT:			return TextureFormat::RG_32UI;
		case SPV_REFLECT_FORMAT_R16_UINT:
		case SPV_REFLECT_FORMAT_R16_SINT:
		case SPV_REFLECT_FORMAT_R16_SFLOAT:
		case SPV_REFLECT_FORMAT_R16G16_UINT:
		case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:
		case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:
		case SPV_REFLECT_FORMAT_R32G32_SINT:
		case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
		case SPV_REFLECT_FORMAT_R32G32B32_UINT:
		case SPV_REFLECT_FORMAT_R32G32B32_SINT:
		case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
		case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
		case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
		case SPV_REFLECT_FORMAT_R64_SINT:
		case SPV_REFLECT_FORMAT_R64_SFLOAT:
		case SPV_REFLECT_FORMAT_R64G64_UINT:
		case SPV_REFLECT_FORMAT_R64G64_SINT:
		case SPV_REFLECT_FORMAT_R64G64_SFLOAT:
		case SPV_REFLECT_FORMAT_R64G64B64_UINT:
		case SPV_REFLECT_FORMAT_R64G64B64_SINT:
		case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:
		case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:
		case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:
		case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT:
		case SPV_REFLECT_FORMAT_R16G16_SINT:
		case SPV_REFLECT_FORMAT_R16G16_SFLOAT:
		case SPV_REFLECT_FORMAT_R16G16B16_UINT:
		case SPV_REFLECT_FORMAT_R16G16B16_SINT:
		case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:
		case SPV_REFLECT_FORMAT_UNDEFINED:
		default:
			assert(false && "Unsupported texture format");
			break;
		}
	}
}