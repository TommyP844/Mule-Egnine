#pragma once

#include "Graphics/RenderTypes.h"

#include <map>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Mule
{
	class IVulkanShader
	{
	public:
		IVulkanShader(VkDevice device);
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

	private:
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& source);
	};
}