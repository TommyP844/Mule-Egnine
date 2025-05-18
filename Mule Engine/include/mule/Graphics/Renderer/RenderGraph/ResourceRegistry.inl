#include "ResourceRegistry.h"
#pragma once


namespace Mule
{
	template<class T, typename... Args>
	inline void ResourceRegistry::AddResource(const std::string& name, ResourceType type, Args&&... args)
	{
		InFlightResource resource;
		resource.Resources.resize(mFramesInFlight);
		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			resource.Resources[i] = T::Create(std::forward<Args>(args)...);
		}

		ResourceHandle handle = ResourceHandle(name, type);
		mResources[handle] = resource;
		mResourceHandles.push_back(handle);
	}

	template<class T>
	inline void ResourceRegistry::InsertResources(ResourceHandle handle, const std::vector<Ref<T>>& resources)
	{
		assert(mFramesInFlight == resources.size() && "resources.size() != mFramesInFlight");

		InFlightResource inFlightResource(mFramesInFlight);

		for (uint32_t i = 0; i < mFramesInFlight; i++)
		{
			inFlightResource.Resources[i] = resources[i];
		}

		mResources[handle] = inFlightResource;
		mResourceHandles.push_back(handle);
	}

	template<class T>
	inline Ref<T> ResourceRegistry::GetResource(ResourceHandle handle, uint32_t frameIndex) const
	{
		auto iter = mResources.find(handle);
		assert(iter != mResources.end() && "Failed to find handle");

		return std::get<Ref<T>>(iter->second.Resources[frameIndex]);
	}

	template<typename T>
	inline T ResourceRegistry::GetVariable(RegistryVariable var, uint32_t frameIndex) const
	{
		switch (var)
		{
		case Mule::RegistryVariable::Width: return mResizeRequests[frameIndex].Width;
		case Mule::RegistryVariable::Height: return mResizeRequests[frameIndex].Height;
		case Mule::RegistryVariable::MAX:
		default:
			assert(false && "Invalid registry variable");
			break;
		}
	}
}