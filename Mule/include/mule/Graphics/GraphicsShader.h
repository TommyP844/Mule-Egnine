#pragma once

// Engine
#include "WeakRef.h"
#include "Ref.h"
#include "RenderPass.h"
#include "VertexLayout.h"
#include "RenderTypes.h"
#include "DescriptorSetLayout.h"
#include "Asset/Asset.h"

#include <vulkan/vulkan.h>

// STD
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

namespace Mule
{
	enum class CullMode {
		Back = VK_CULL_MODE_BACK_BIT,
		Front = VK_CULL_MODE_FRONT_BIT,
		None = VK_CULL_MODE_NONE
	};

	struct PushConstant
	{
		PushConstant() = default;
		PushConstant(ShaderStage stage, uint32_t size)
			:
			Stage(stage),
			Size(size)
		{}
		ShaderStage Stage;
		uint32_t Size;
	};

	struct GraphicsShaderDescription
	{
		fs::path SourcePath;
		uint32_t Subpass;
		Ref<RenderPass> RenderPass = nullptr;
		VertexLayout VertexLayout;
		std::vector<WeakRef<DescriptorSetLayout>> DescriptorLayouts;
		std::vector<PushConstant> PushConstants;
		CullMode CulleMode = CullMode::Back;
		bool EnableDepthTesting = true;
		bool WriteDepth = true;
	};

	class GraphicsShader : public Asset<AssetType::Shader>
	{
	public:
		GraphicsShader(VkDevice device, const GraphicsShaderDescription& description);
		~GraphicsShader();

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }
		const std::pair<uint32_t, uint32_t>& GetPushConstantRange(ShaderStage stage);

		bool IsValid() const { return mIsValid; }
	private:
		VkDevice mDevice;
		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;
		bool mIsValid;

		// <uint32_t, uint32_t> || <offset, size>
		std::map<ShaderStage, std::pair<uint32_t, uint32_t>> mPushConstantMapping;

		bool Compile(const fs::path& sourcePath, std::vector<VkPipelineShaderStageCreateInfo>& stages);
		VkPipelineShaderStageCreateInfo CreateStage(const std::vector<uint32_t>& source, ShaderStage stage);
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& source);
	};
}
