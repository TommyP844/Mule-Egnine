#pragma once

// Engine
#include "Ref.h"
#include "RenderPass.h"
#include "VertexLayout.h"
#include "RenderTypes.h"

#include <vulkan/vulkan.h>

// STD
#include <filesystem>

namespace fs = std::filesystem;

namespace Mule
{
	struct GraphicsShaderDescription
	{
		fs::path SourcePath;
		uint32_t Subpass;
		Ref<RenderPass> RenderPass = nullptr;
		VertexLayout VertexLayout;
	};

	class GraphicsShader
	{
	public:
		GraphicsShader(VkDevice device, const GraphicsShaderDescription& description);
		~GraphicsShader();

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

		bool IsValid() const { return mIsValid; }
	private:
		VkDevice mDevice;
		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;
		bool mIsValid;

		bool Compile(const fs::path& sourcePath, std::vector<VkPipelineShaderStageCreateInfo>& stages);
		VkPipelineShaderStageCreateInfo CreateStage(const std::vector<uint32_t>& source, ShaderStage stage);
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& source);
	};
}
