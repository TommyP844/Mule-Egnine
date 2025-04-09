
#include "Graphics/Shader/IVulkanShader.h"

#include "Core/StringUtil.h"

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
			else if (line == "#COMPUTE")
			{
				currentStage = ShaderStage::Compute;
				continue;
			}

			if (currentStage == ShaderStage::None)
				continue;

			sources[currentStage] += line + "\n";
		}
		file.close();

		return sources;
	}
	
	bool IVulkanShader::Load(const fs::path& sourcePath, std::vector<VkPipelineShaderStageCreateInfo>& stages, const std::vector<std::pair<std::string, std::string>>& macros)
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
				SPDLOG_ERROR("Failed to get reflection data for stage: {}, in shader: {}", (uint32_t)stage, sourcePath.string());
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

	void IVulkanShader::ParseOptions(std::string options)
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

	void IVulkanShader::LoadOption(const std::string& option, const std::string& value)
	{
		if (option == ToLower("CullMode"))
		{
			if (value == ToLower("Front"))
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

	AttributeType IVulkanShader::GetAttributeTypeFromSPVFormat(SpvReflectFormat format)
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

	DescriptorType IVulkanShader::GetDescriptorTypeFromSPVFormat(SpvReflectDescriptorType type)
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
		case ShaderStage::Compute: kind = shaderc_shader_kind::shaderc_compute_shader; break;
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