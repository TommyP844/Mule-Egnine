#include "Graphics/ShaderFactory.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	ShaderFactory::ShaderFactory()
	{
	}

	ShaderFactory::~ShaderFactory()
	{
	}

	void ShaderFactory::RegisterGraphicsPipeline(const std::string& name, const GraphicsPipelineDescription& description)
	{
		assert(mGraphicsPipelineDescriptions.find(name) == mGraphicsPipelineDescriptions.end() 
			&& "Pipeline name has already been registered");
		mGraphicsPipelineDescriptions[name] = description;
	}

	void ShaderFactory::RegisterComputePipeline(const std::string& name, const ComputePipelineDescription& description)
	{
		assert(mComputePipelineDescriptions.find(name) == mComputePipelineDescriptions.end() 
			&& "Pipeline name has already been registered");
		mComputePipelineDescriptions[name] = description;
	}

	WeakRef<GraphicsPipeline> ShaderFactory::GetOrCreateGraphicsPipeline(const std::string& name)
	{
		auto iter = mGraphicsPipelines.find(name);
		if (iter == mGraphicsPipelines.end())
		{
			assert(mGraphicsPipelineDescriptions.find(name) != mGraphicsPipelineDescriptions.end()
				&& "Graphics Pipeline name not registered");

			auto pipeline = GraphicsPipeline::Create(mGraphicsPipelineDescriptions[name]);
			mGraphicsPipelines[name] = pipeline;

			return pipeline;
		}

		return iter->second;
	}

	WeakRef<ComputePipeline> ShaderFactory::GetOrCreateComputePipeline(const std::string& name)
	{
		auto iter = mComputePipelines.find(name);
		if (iter == mComputePipelines.end())
		{
			assert(mComputePipelineDescriptions.find(name) != mComputePipelineDescriptions.end()
				&& "Compute Pipeline name not registered");

			auto pipeline = ComputePipeline::Create(mComputePipelineDescriptions[name]);
			mComputePipelines[name] = pipeline;

			return pipeline;
		}

		return iter->second;
	}

	void ShaderFactory::UnloadGraphicsPipeline(const std::string& name)
	{
		auto iter = mGraphicsPipelines.find(name);
		if (iter == mGraphicsPipelines.end())
		{
			SPDLOG_WARN("Trying to unload graphics pipeline that does not exist: {}", name);
			return;
		}

		mGraphicsPipelines.erase(name);
	}

	void ShaderFactory::UnloadComputePipeline(const std::string& name)
	{
		auto iter = mGraphicsPipelines.find(name);
		if (iter == mGraphicsPipelines.end())
		{
			SPDLOG_WARN("Trying to unload compute pipeline that does not exist: {}", name);
			return;
		}

		mGraphicsPipelines.erase(name);
	}
}