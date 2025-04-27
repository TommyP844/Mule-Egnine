#pragma once

#include "Ref.h"
#include "Graphics/API/ShaderResourceBlueprint.h"

#include <filesystem>
#include <vector>

namespace Mule
{
	struct ComputePipelineDescription
	{
		std::filesystem::path Filepath;
		std::vector<Ref<ShaderResourceBlueprint>> Resources;
		uint32_t ConstantBufferSize;
	};

	class ComputePipeline
	{
	public:
		static Ref<ComputePipeline> Create(const ComputePipelineDescription& description);

		virtual ~ComputePipeline() = default;
		Ref<ShaderResourceBlueprint> GetBlueprintIndex(uint32_t index) const { return mBlueprints[index]; }
		uint32_t GetBlueprintCount() const { return mBlueprints.size(); }

	protected:
		ComputePipeline() = default;

		std::vector<Ref<ShaderResourceBlueprint>> mBlueprints;
	};
}
