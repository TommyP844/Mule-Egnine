#pragma once

// Engine
#include "WeakRef.h"
#include "Ref.h"
#include "Graphics/RenderPass.h"
#include "Graphics/VertexLayout.h"
#include "Graphics/RenderTypes.h"
#include "Graphics/DescriptorSetLayout.h"
#include "Asset/Asset.h"
#include "IVulkanShader.h"

#include <../Source/SPIRV-Reflect/spirv_reflect.h>

#include <Volk/volk.h>

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
		uint32_t AttachmentCount = 0;
	};

	class GraphicsShader : public Asset<AssetType::Shader>, public IVulkanShader
	{
	public:
		GraphicsShader(WeakRef<GraphicsContext> context, const fs::path& filepath);
		~GraphicsShader();

		void Reload();

		const std::pair<uint32_t, uint32_t>& GetPushConstantRange(ShaderStage stage);

		bool IsValid() const { return mIsValid; }
		const std::vector<uint32_t>& AttachmentLocations() const { return mOptions.AttachmentLocations; }
	private:
		bool mIsValid;
		WeakRef<GraphicsContext> mContext;

		// <uint32_t, uint32_t> || <offset, size>
		std::map<ShaderStage, std::pair<uint32_t, uint32_t>> mPushConstantMapping;

		struct ShaderAttachment
		{
			TextureFormat Format;
			bool BlendEnable;
			uint32_t Location;
		};

		struct ShaderOptions
		{
			VertexLayout VertexLayout;
			std::vector<Ref<DescriptorSetLayout>> DescriptorLayouts;
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

	private:
		bool Load(const fs::path& sourcePath, std::vector<VkPipelineShaderStageCreateInfo>& stages, const std::vector<std::pair<std::string, std::string>>& macros = {});

		void ParseOptions(std::string options);
		void LoadOption(const std::string& option, const std::string& value);

		AttributeType GetAttributeTypeFromSPVFormat(SpvReflectFormat format);
		DescriptorType GetDescriptorTypeFromSPVFormat(SpvReflectDescriptorType type);
	};
}
