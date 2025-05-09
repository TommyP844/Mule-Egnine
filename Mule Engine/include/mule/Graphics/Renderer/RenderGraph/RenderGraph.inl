#pragma once

#include "Graphics/Renderer/RenderGraph/RenderPasses/RenderPass.h"

namespace Mule
{
	template<typename T, typename... Args>
	inline WeakRef<T> RenderGraph::CreatePass(ResourceBuilder& resourceBuilder, Args&&... args)
	{
		if (!std::is_base_of<RenderPass, T>())
			assert("Type does not inherit from RenderPass");

		auto pass = MakeRef<T>(resourceBuilder, std::forward<Args>(args)...);
		mPassesToCompile.push_back(pass);

		return pass;
	}
}
