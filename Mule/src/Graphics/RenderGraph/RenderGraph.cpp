#include "Graphics/RenderGraph/RenderGraph.h"

#include <spdlog/spdlog.h>

namespace Mule::RenderGraph
{
	RenderGraph::RenderGraph()
		:
		mIsValid(false)
	{
	}
	
	RenderGraph::~RenderGraph()
	{
	}
	
	void RenderGraph::AddResource(const std::string& name, Ref<IResource> resource)
	{
		auto iter = mResources.find(name);
		if (iter != mResources.end())
		{
			SPDLOG_WARN("RenderGraph Resource already exists: {}", name);
			return;
		}
		mResources[name] = resource;
	}
	
	void RenderGraph::AddPass(const std::string& name, const std::vector<std::string>& inputs, const std::vector<std::string>& outputs, std::function<void(PassContext&)> callback)
	{
		auto iter = mPassesToCompile.find(name);
		if (iter != mPassesToCompile.end())
		{
			SPDLOG_WARN("RenderGraph Pass already exists: {}", name);
			return;
		}
		
		RenderPass& renderPass = mPassesToCompile[name];
		renderPass.Inputs = inputs;
		renderPass.Outputs = outputs;
		renderPass.CallBack = callback;
	}
	
	void RenderGraph::Compile()
	{
	}
	
	void RenderGraph::Execute()
	{
		if (!mIsValid)
		{
			SPDLOG_WARN("Invalid Render Graph trying to execute");
			return;
		}
	}
}