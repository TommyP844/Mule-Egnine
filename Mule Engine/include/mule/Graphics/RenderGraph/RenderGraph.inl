#include "RenderGraph.h"
#pragma once

namespace Mule::RenderGraph
{
	template<typename T>
	inline WeakRef<T> RenderGraph::CreatePass()
	{
		if (!std::is_base_of<IRenderPass, T>())
			assert("Type does not inherit from IRenderpass");

		auto pass = MakeRef<T>(mServiceManager, this);
		mPassesToCompile.push_back(pass);

		return pass;
	}

	template<class T, typename ...Args>
	inline ResourceHandle<T> RenderGraph::AddResource(Args && ...args)
	{
		ResourceHandle<T> handle = ResourceHandle<T>::Create();
		InFlightResource<T> resource;
		resource.Resources.resize(mFramesInFlight);
		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			resource.Resources[i] = T::Create(std::forward<Args>(args)...);
		}

		auto& map = GetResourceMap<T>();
		map[handle] = resource;

		return handle;
	}

	template<typename T>
	inline Ref<T> Mule::RenderGraph::RenderGraph::GetResource(ResourceHandle<T> handle) const
	{
		auto& map = GetResourceMap<T>();
		auto iter = map.find(handle);
		assert(iter != map.end() && "Failed to find handle");

		return iter->second.Resources[mFrameIndex];
	}
}
