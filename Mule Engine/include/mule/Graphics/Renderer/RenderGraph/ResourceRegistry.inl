#pragma once


namespace Mule
{
	template<class T, typename... Args>
	inline void ResourceRegistry::AddResource(ResourceHandle handle, Args&&... args)
	{
		InFlightResource resource;
		resource.Resources.resize(mFramesInFlight);
		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			resource.Resources[i] = T::Create(std::forward<Args>(args)...);
		}

		mResources[handle] = resource;
	}

	template<class T>
	inline Ref<T> ResourceRegistry::GetResource(ResourceHandle handle, uint32_t frameIndex) const
	{
		auto iter = mResources.find(handle);
		assert(iter != mResources.end() && "Failed to find handle");

		return std::get<Ref<T>>(iter->second.Resources[frameIndex]);
	}
}