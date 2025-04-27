#include "Graphics/RenderGraph/IRenderPass.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#include <assert.h>

namespace Mule::RenderGraph
{
	IRenderPass::IRenderPass(const std::string& name, WeakRef<RenderGraph> graph)
		:
		mName(name),
		mGraph(graph)
	{
	}

	void IRenderPass::AddPassDependency(const std::string& dependentPass)
	{
		auto iter = std::find(mDependencies.begin(), mDependencies.end(), dependentPass);
		if (iter != mDependencies.end())
		{
			std::string err = "Dependency already added to pass: " + mName + ", Dependency: " + dependentPass;
			assert(false && err.c_str());
		}

		mDependencies.push_back(dependentPass);
	}
}
