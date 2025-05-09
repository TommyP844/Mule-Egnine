#include "Graphics/Renderer/RenderGraph/RenderPasses/RenderPass.h"

#include <assert.h>

namespace Mule
{
	RenderPass::RenderPass(const std::string& name, PassType type)
		:
		mName(name),
		mPassType(type)
	{
	}

	void RenderPass::AddPassDependency(const std::string& dependentPass)
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
