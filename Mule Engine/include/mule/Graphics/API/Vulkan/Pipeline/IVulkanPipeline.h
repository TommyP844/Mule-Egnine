#pragma once

#include "Graphics/API/GraphicsCore.h"

#include "Graphics/API/Vulkan/VulkanDescriptorSetLayout.h"

#include "Graphics/API/GraphicsCore.h"

#include <volk/volk.h>

#include <vector>
#include <string>
#include <map>
#include <filesystem>

#include <shaderc/shaderc.h>
#include <../Source/SPIRV-Reflect/spirv_reflect.h>

namespace Mule::Vulkan
{
	struct PushConstantInfo
	{
		ShaderStage Stage;
		uint32_t Offset;
		uint32_t Size;
	};

	class IVulkanPipeline
	{
	public:
		virtual ~IVulkanPipeline() = default;

	protected:
		IVulkanPipeline() = default;

		struct OutputAttachment
		{
			uint32_t Location;
			TextureFormat Format;
		};

		struct ReflectionData
		{
			std::vector<Ref<VulkanDescriptorSetLayout>> Layouts;
			std::vector<PushConstantInfo> PushConstants;
			std::vector<OutputAttachment> Attachments;
		};

		bool Compile(const std::filesystem::path& filepath, std::map<ShaderStage, std::vector<uint32_t>>& byteCodes, const std::vector<std::pair<std::string, std::string>>& macros);

		ReflectionData Reflect(std::map<ShaderStage, std::vector<uint32_t>>& byteCodes);

		std::vector<VkPipelineShaderStageCreateInfo> BuildShaderStages(const std::map<ShaderStage, std::vector<uint32_t>>& byteCodes);

	private:
		bool CompileShaderStage(shaderc_compiler_t compiler, shaderc_compile_options_t options, shaderc_shader_kind kind, std::vector<uint32_t>& byteCode, const std::string& codeText, const char* filename);
		ShaderResourceType GetResourceType(SpvReflectDescriptorType type);
		TextureFormat GetTextureFormat(SpvReflectFormat format);
	};
}