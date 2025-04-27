#pragma once

#include "Ref.h"
#include "WeakRef.h"

#include "Graphics/API/GraphicsPipeline.h"
#include "Graphics/API/ComputePipeline.h"

#include <unordered_map>
#include <string>

namespace Mule
{
	class ShaderFactory
	{
	public:
		ShaderFactory();
		~ShaderFactory();
						
		void RegisterGraphicsPipeline(const std::string& name, const GraphicsPipelineDescription& description);
		void RegisterComputePipeline(const std::string& name, const ComputePipelineDescription& description);

		WeakRef<GraphicsPipeline> GetOrCreateGraphicsPipeline(const std::string& name);
		WeakRef<ComputePipeline> GetOrCreateComputePipeline(const std::string& name);

		void UnloadGraphicsPipeline(const std::string& name);
		void UnloadComputePipeline(const std::string& name);

	private:
		std::unordered_map<std::string, GraphicsPipelineDescription> mGraphicsPipelineDescriptions;
		std::unordered_map<std::string, ComputePipelineDescription> mComputePipelineDescriptions;

		std::unordered_map<std::string, Ref<GraphicsPipeline>> mGraphicsPipelines;
		std::unordered_map<std::string, Ref<ComputePipeline>> mComputePipelines;
	};
}
