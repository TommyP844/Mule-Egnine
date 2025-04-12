#pragma once

#include "VulkanRenderTypes.h"
#include "VulkanDescriptorSetLayout.h"
#include "Graphics/VertexLayout.h"

#include <map>
#include <vector>
#include <string>
#include <filesystem>

#include <../Source/SPIRV-Reflect/spirv_reflect.h>


namespace fs = std::filesystem;

namespace Mule
{
	enum class CullMode {
		Back = VK_CULL_MODE_BACK_BIT,
		Front = VK_CULL_MODE_FRONT_BIT,
		None = VK_CULL_MODE_NONE
	};

	enum class FillMode {
		Solid = VK_POLYGON_MODE_FILL,
		Wireframe = VK_POLYGON_MODE_LINE,
		Point = VK_POLYGON_MODE_POINT
	};

	struct PushConstant
	{
		PushConstant() = default;
		PushConstant(ShaderStage stage, uint32_t size)
			:
			Stage(stage),
			Size(size)
		{
		}
		ShaderStage Stage;
		uint32_t Size;
	};

	class IVulkanShader
	{
	public:
		IVulkanShader();
		virtual ~IVulkanShader() = default;

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	protected:
		std::map<ShaderStage, std::string> Parse(const fs::path& sourcePath);
		std::vector<uint32_t> Compile(ShaderStage stage, const std::string& source, const fs::path& filepath, const std::vector<std::pair<std::string, std::string>>& macros = {});
		VkPipelineShaderStageCreateInfo CreateStage(const std::vector<uint32_t>& source, ShaderStage stage);

		VkDevice mDevice;
		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;

		struct ShaderAttachment
		{
			TextureFormat Format;
			bool BlendEnable;
			uint32_t Location;
		};

		struct ShaderOptions
		{
			VertexLayout VertexLayout;
			std::vector<Ref<VulkanDescriptorSetLayout>> DescriptorLayouts;
			std::vector<PushConstant> PushConstants;
			CullMode CulleMode = CullMode::Back;
			bool EnableDepthTesting = true;
			bool WriteDepth = true;
			bool BlendEnable = false;
			FillMode FillMode = FillMode::Solid;
			float LineWidth = 1.0f;
			std::vector<ShaderAttachment> Attachments;
			std::vector<uint32_t> AttachmentLocations;
			// Set, Layout
			std::map<uint32_t, std::map<uint32_t, LayoutDescription>> DescriptorSetLayoutDescriptions;
			TextureFormat DepthFormat;
		};

		ShaderOptions mOptions;

		// <uint32_t, uint32_t> || <offset, size>
		std::map<ShaderStage, std::pair<uint32_t, uint32_t>> mPushConstantMapping;

		bool Load(const fs::path& sourcePath, std::vector<VkPipelineShaderStageCreateInfo>& stages, const std::vector<std::pair<std::string, std::string>>& macros = {});

		void ParseOptions(std::string options);
		void LoadOption(const std::string& option, const std::string& value);

		AttributeType GetAttributeTypeFromSPVFormat(SpvReflectFormat format);
		DescriptorType GetDescriptorTypeFromSPVFormat(SpvReflectDescriptorType type);

	private:
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& source);
	};
}