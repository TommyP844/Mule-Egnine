#pragma once

#include "Ref.h"
#include "Graphics/VertexLayout.h"
#include "Graphics/API/GraphicsCore.h"
#include "Graphics/API/ShaderResourceBlueprint.h"

#include <filesystem>

namespace Mule
{
	struct ShaderAttachment
	{
		TextureFormat Format;
		uint32_t Location;
	};

	struct GraphicsPipelineDescription
	{
		std::filesystem::path Filepath;
		FillMode FilleMode;
		CullMode CullMode;
		VertexLayout VertexLayout;
		TextureFormat DepthFormat;
		bool EnableDepthTest = true;
		bool EnableDepthWrite = true;
		float LineWidth = 1.f;
		bool EnableBlending = false;
	};

	class GraphicsPipeline
	{
	public:
		static Ref<GraphicsPipeline> Create(const GraphicsPipelineDescription& description);

		virtual ~GraphicsPipeline() = default;

		virtual void Reload() = 0;

		const std::vector<ShaderAttachment>& GetOutputAttachments() const { return mOutputAttachemnts; }
		Ref<ShaderResourceBlueprint> GetBlueprintIndex(uint32_t index) const { return mBlueprints[index]; }
		uint32_t GetBlueprintCount() const { return mBlueprints.size(); }

	protected:
		std::vector<ShaderAttachment> mOutputAttachemnts;
		std::vector<Ref<ShaderResourceBlueprint>> mBlueprints;
	};
}
