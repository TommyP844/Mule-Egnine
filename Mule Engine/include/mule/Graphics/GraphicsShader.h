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
		{}
		ShaderStage Stage;
		uint32_t Size;
	};

	struct GraphicsShaderDescription
	{
		fs::path SourcePath;
		uint32_t Subpass;
		WeakRef<RenderPass> RenderPass = nullptr;
		VertexLayout VertexLayout;
		std::vector<WeakRef<DescriptorSetLayout>> DescriptorLayouts;
		std::vector<PushConstant> PushConstants;
		CullMode CulleMode = CullMode::Back;
		bool EnableDepthTesting = true;
		bool WriteDepth = true;
		std::vector<std::pair<std::string, std::string>> Macros = {};
		bool BlendEnable = false;
		FillMode FillMode = FillMode::Solid;
		float LineWidth = 1.0f;
	};

	class GraphicsShader : public Asset<AssetType::Shader>
	{
	public:
		GraphicsShader(VkDevice device, const GraphicsShaderDescription& description);
		~GraphicsShader();

		void Reload();

		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }
		const std::pair<uint32_t, uint32_t>& GetPushConstantRange(ShaderStage stage);

		bool IsValid() const { return mIsValid; }
	private:
		VkDevice mDevice;
		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;
		bool mIsValid;
		GraphicsShaderDescription mDescription;

		// <uint32_t, uint32_t> || <offset, size>
		std::map<ShaderStage, std::pair<uint32_t, uint32_t>> mPushConstantMapping;

		bool Compile(const fs::path& sourcePath, std::vector<VkPipelineShaderStageCreateInfo>& stages, const std::vector<std::pair<std::string, std::string>>& macros = {});
		VkPipelineShaderStageCreateInfo CreateStage(const std::vector<uint32_t>& source, ShaderStage stage);
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& source);
	};
}
